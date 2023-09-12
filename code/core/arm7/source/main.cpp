#include <nds.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosEvent.h>
#include <libtwl/sound/sound.h>
#include <libtwl/sound/soundChannel.h>
#include <libtwl/sio/sio.h>
#include <libtwl/gfx/gfxStatus.h>

static rtos_event_t sVBlankEvent;

static void vblankIrq(u32 irqMask)
{
    rtos_signalEvent(&sVBlankEvent);
}

static void notifyArm7Ready()
{
    ipc_setArm7SyncBits(7);
}

int main()
{
    rtos_initIrq();
    rtos_startMainThread();
    ipc_initFifoSystem();

    // clear sound registers
    dmaFillWords(0, (void*)&REG_SOUNDxCNT(0), 0x100);

    writePowerManagement(PM_CONTROL_REG, (readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE) | PM_SOUND_AMP);
    powerOn(POWER_SOUND);

    readUserSettings();
    ledBlink(0);

    sio_setGpioSiIrq(false);
    sio_setGpioMode(RCNT0_L_MODE_GPIO);

    snd_setMasterVolume(127);
    snd_setMasterEnable(true);

    rtos_createEvent(&sVBlankEvent);
    rtos_setIrqFunc(RTOS_IRQ_VBLANK, vblankIrq);
    rtos_enableIrqMask(RTOS_IRQ_VBLANK);
    gfx_setVBlankIrqEnabled(true);

    notifyArm7Ready();

    while (true)
    {
        rtos_waitEvent(&sVBlankEvent, true, true);
    }
    
    return 0;
}
