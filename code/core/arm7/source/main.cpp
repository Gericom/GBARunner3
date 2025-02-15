#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/sys/sysPower.h>
#include <libtwl/mem/memSwap.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosEvent.h>
#include <libtwl/sound/sound.h>
#include <libtwl/sound/soundChannel.h>
#include <libtwl/sio/sio.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/i2c/i2cMcu.h>
#include <libtwl/spi/spiPmic.h>
#include "IpcServices/FsIpcService.h"
#include "IpcServices/GbaSoundIpcService.h"
#include "IpcServices/SystemIpcService.h"
#include "IpcServices/GbaSaveIpcService.h"
#include "Arm7State.h"
#include "ExitMode.h"
#include "FramerateAdjustment.h"
#include "mmc/tmio.h"

static FsIpcService sFsIpcService;
static GbaSoundIpcService sGbaSoundIpcService;
static SystemIpcService sSystemIpcService;
static GbaSaveIpcService sGbaSaveIpcService;
static rtos_event_t sVBlankEvent;
static volatile u8 sMcuIrqFlag = false;
static Arm7State sState;
static ExitMode sExitMode;

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
        sExitMode = ExitMode::Reset;
        sState = Arm7State::ExitRequested;
    }
    else if (irqMask & MCU_IRQ_POWER_OFF)
    {
        // power button was held long to trigger a power off
        sExitMode = ExitMode::PowerOff;
        sState = Arm7State::ExitRequested;
    }
}

static void notifyArm7Ready()
{
    ipc_setArm7SyncBits(7);
}

static void initializeIpcServices()
{
    sFsIpcService.Start();
    sGbaSoundIpcService.Start();
    sSystemIpcService.Start();
    sGbaSaveIpcService.Start();
}

static void initializeVBlankIrq()
{
    rtos_createEvent(&sVBlankEvent);
    rtos_setIrqFunc(RTOS_IRQ_VBLANK, vblankIrq);
    rtos_enableIrqMask(RTOS_IRQ_VBLANK);
    gfx_setVBlankIrqEnabled(true);
}

static void initializeArm7()
{
    rtos_initIrq();
    rtos_startMainThread();

    while (ipc_getArm9SyncBits() != 0);

    ipc_initFifoSystem();

    // clear sound registers
    dmaFillWords(0, (void*)&REG_SOUNDxCNT(0), 0x100);

    pmic_setAmplifierEnable(true);
    sys_setSoundPower(true);

    readUserSettings();
    pmic_setPowerLedBlink(PMIC_CONTROL_POWER_LED_BLINK_NONE);

    sio_setGpioSiIrq(false);
    sio_setGpioMode(RCNT0_L_MODE_GPIO);

    if (isDSiMode())
    {
        TMIO_init();
    }
    
    initializeIpcServices();

    snd_setMasterVolume(127);
    snd_setMasterEnable(true);

    initializeVBlankIrq();

    if (isDSiMode())
    {
        rtos_setIrq2Func(RTOS_IRQ2_MCU, mcuIrq);
        rtos_enableIrq2Mask(RTOS_IRQ2_MCU);
    }

    fps_initializeFramerateAdjustment();
    fps_startFramerateAdjustment();

    notifyArm7Ready();
}

static void updateArm7IdleState()
{
    checkMcuIrq();
    sGbaSaveIpcService.Update();

    if (sState == Arm7State::ExitRequested)
    {
        snd_setMasterVolume(0); // mute sound
    }
}

static bool performExit(ExitMode exitMode)
{
    switch (exitMode)
    {
        case ExitMode::Reset:
        {
            mcu_setWarmBootFlag(true);
            mcu_hardReset();
            break;
        }
        case ExitMode::PowerOff:
        {
            pmic_shutdown();
            break;
        }
    }

    while (true); // wait infinitely for exit
}

static void updateArm7ExitRequestedState()
{
    if (sGbaSaveIpcService.FlushSaveIfDirty())
    {
        performExit(sExitMode);
    }
}

static void updateArm7()
{
    switch (sState)
    {
        case Arm7State::Idle:
        {
            updateArm7IdleState();
            break;
        }
        case Arm7State::ExitRequested:
        {
            updateArm7ExitRequestedState();
            break;
        }
    }
}

int main()
{
    initializeArm7();

    sState = Arm7State::Idle;
    while (true)
    {
        rtos_waitEvent(&sVBlankEvent, true, true);
        updateArm7();
    }
    
    return 0;
}
