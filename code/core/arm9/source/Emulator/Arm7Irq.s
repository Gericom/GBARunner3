.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_arm7Irq
    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r4,r12,lr}
#ifndef GBAR3_TEST
    ldr r12,= dma_state
    ldr r4, [r12] // dmaFlags
    tst r4, #(1 << 9)
        movne r0, #1
        blne dma_dmaSound
    tst r4, #(1 << 10)
        movne r0, #2
        blne dma_dmaSound
	bl sav_writeSaveToFile
#endif
    pop {r0-r3,r4,r12,pc}
