.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.extern dma_state

/// @brief Performs hblank irq tasks for the emulator.
///        - Emulates HDMA
/// @param r0-r12 Preserved
/// @param r13 Trashed.
/// @param lr Return address.
arm_func emu_hblankIrq
    mov r13, #0x04000000
    ldrh r13, [r13, #6]
    cmp r13, #160
    bxge lr

    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r4,r5,r12,lr}
#ifndef GBAR3_TEST
    ldr r5,= dma_state
    ldr r4, [r5] // dmaFlags
    tst r4, #1
        movne r0, #0
        blne dma_dmaTransfer
    tst r4, #2
        movne r0, #1
        blne dma_dmaTransfer
    tst r4, #4
        movne r0, #2
        blne dma_dmaTransfer
    tst r4, #8
        movne r0, #3
        blne dma_dmaTransfer
#endif
    pop {r0-r3,r4,r5,r12,pc}
