#include <nds.h>

static void vblankIrq(void)
{

}

int main()
{
    // clear sound registers
    dmaFillWords(0, (void*)0x04000400, 0x100);

    REG_SOUNDCNT |= SOUND_ENABLE;
    writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
    powerOn(POWER_SOUND);

    readUserSettings();
    ledBlink(0);

    irqInit();
    fifoInit();
    touchInit();

    installSoundFIFO();

    installSystemFIFO();

    irqSet(IRQ_VBLANK, vblankIrq);

    irqEnable(IRQ_VBLANK);

    while (true)
    {
        swiWaitForVBlank();
    }
    return 0;
}
