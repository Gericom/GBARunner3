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
#include <libtwl/spi/spiPmic.h>
#include "IpcServices/FsIpcService.h"
#include "IpcServices/GbaSoundIpcService.h"
#include "IpcServices/SystemIpcService.h"
#include "FramerateAdjustment.h"

static FsIpcService sFsIpcService;
static GbaSoundIpcService sGbaSoundIpcService;
static SystemIpcService sSystemIpcService;
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

	static u32 irqMask = 0;

    // check the irq mask
	static bool pressed = false;
	if (pressed) {
		if (irqMask & MCU_IRQ_RESET)
		{
			if (ipc_getArm9SyncBits() != 6) return; // return if .sav is still writing

            // power button was released
			mcu_setWarmBootFlag(true);
			mcu_hardReset();

			while (1);
		}
		if (irqMask & MCU_IRQ_POWER_OFF)
		{
			if (ipc_getArm9SyncBits() != 6) return; // return if .sav is still writing

            // power button was held long to trigger a power off
			pmic_shutdown();

			while (1);
		}
	}

    // check and ack the flag atomically
    if (!mem_swapByte(false, &sMcuIrqFlag))
        return;

    irqMask = mcu_getIrqMask();
    if ((irqMask & MCU_IRQ_RESET) || (irqMask & MCU_IRQ_POWER_OFF))
    {
        // todo: maybe ensure no sd writes are still pending
		pressed = true;
		ipc_setArm7SyncBits(6); // notify arm9 to write .sav file
		snd_setMasterVolume(0); // mute sound
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

    pmic_setAmplifierEnable(true);
    powerOn(POWER_SOUND);

    readUserSettings();
    pmic_setPowerLedBlink(PMIC_CONTROL_POWER_LED_BLINK_NONE);

    sio_setGpioSiIrq(false);
    sio_setGpioMode(RCNT0_L_MODE_GPIO);

    sFsIpcService.Start();
    sGbaSoundIpcService.Start();
    sSystemIpcService.Start();

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

    fps_initializeFramerateAdjustment();
    fps_startFramerateAdjustment();

    notifyArm7Ready();

    while (true)
    {
        rtos_waitEvent(&sVBlankEvent, true, true);
        checkMcuIrq();
    }
    
    return 0;
}
