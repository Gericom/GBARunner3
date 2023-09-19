#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/mem/memSwap.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosEvent.h>
#include <libtwl/sound/sound.h>
#include <libtwl/sound/soundChannel.h>
#include <libtwl/sio/sio.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/i2c/i2cMcu.h>
#include "IpcServices/FsIpcService.h"
#include "IpcServices/GbaSoundIpcService.h"

static FsIpcService sFsIpcService;
static GbaSoundIpcService sGbaSoundIpcService;
static rtos_event_t sVBlankEvent;
static volatile u8 sMcuIrqFlag = false;

static void vblankIrq(u32 irqMask)
{
    rtos_signalEvent(&sVBlankEvent);
}

static void mcuIrq(u32 irq2Mask)
{
    sMcuIrqFlag = true;
}

static void checkMcuIrq(void)
{
    // mcu only exists in DSi mode
    if (!isDSiMode())
        return;

    // check and ack the flag atomically
    if (!mem_swapByte(false, &sMcuIrqFlag))
        return;

    // check the irq mask
    u32 irqMask = mcu_getIrqMask();
    if (irqMask & MCU_IRQ_RESET)
    {
        // power button was released
        // todo: maybe ensure no sd writes are still pending

        mcu_setWarmBootFlag(true);
        mcu_hardReset();

        while (1);
    }
    if (irqMask & MCU_IRQ_POWER_OFF)
    {
        // power button was held long to trigger a power off
        // todo: maybe ensure no sd writes are still pending
        // todo: implement pmic support in libtwl
        writePowerManagement(PM_CONTROL_REG, PM_SYSTEM_PWR);

        while (1);
    }
}

static void notifyArm7Ready()
{
    ipc_setArm7SyncBits(7);
}

int main()
{
    rtos_initIrq();
    rtos_startMainThread();

    while (ipc_getArm9SyncBits() != 0);

    ipc_initFifoSystem();

    // clear sound registers
    dmaFillWords(0, (void*)&REG_SOUNDxCNT(0), 0x100);

    writePowerManagement(PM_CONTROL_REG, (readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE) | PM_SOUND_AMP);
    powerOn(POWER_SOUND);

    readUserSettings();
    ledBlink(0);

    sio_setGpioSiIrq(false);
    sio_setGpioMode(RCNT0_L_MODE_GPIO);

    sFsIpcService.Start();
    sGbaSoundIpcService.Start();

    snd_setMasterVolume(127);
    snd_setMasterEnable(true);

    rtos_createEvent(&sVBlankEvent);
    rtos_setIrqFunc(RTOS_IRQ_VBLANK, vblankIrq);
    rtos_enableIrqMask(RTOS_IRQ_VBLANK);
    gfx_setVBlankIrqEnabled(true);

    if (isDSiMode())
    {
        rtos_setIrq2Func(RTOS_IRQ2_MCU, mcuIrq);
        rtos_enableIrq2Mask(RTOS_IRQ2_MCU);
    }

    notifyArm7Ready();

    while (true)
    {
        rtos_waitEvent(&sVBlankEvent, true, true);
        checkMcuIrq();
    }
    
    return 0;
}
