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
    ldr r5,= dma_state
    ldr r4, [r5] // dmaFlags
    tst r4, #1
    ldrne r3, [r5, #(4 + 0 * 3 * 4 + 8)]
    blxne r3
    tst r4, #2
    ldrne r3, [r5, #(4 + 1 * 3 * 4 + 8)]
    blxne r3
    tst r4, #4
    ldrne r3, [r5, #(4 + 2 * 3 * 4 + 8)]
    blxne r3
    tst r4, #8
    ldrne r3, [r5, #(4 + 3 * 3 * 4 + 8)]
    blxne r3
    pop {r0-r3,r4,r5,r12,pc}
    