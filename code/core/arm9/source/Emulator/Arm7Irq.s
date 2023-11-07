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
    blne dma_dmaSound1
    tst r4, #(1 << 10)
    blne dma_dmaSound2
#endif
    pop {r0-r3,r4,r12,pc}
