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
    ldr r12,= gGbaSioShared + 0xA
    mov r4, #0
    mcr p15, 0, r4, c7, c10, 4 // drain write buffer
    mcr p15, 0, r12, c7, c6, 1 // invalidate range
    swpb lr, r4, [r12]
    cmp lr, #0
        blne sio_finishTransfer
#endif
    pop {r0-r3,r4,r12,pc}
