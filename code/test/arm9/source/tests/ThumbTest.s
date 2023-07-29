.section ".itcm", "ax"

#include "AsmMacros.inc"

old_context:
    .space 0x40

new_context:
    .space 0x40

// r0 = instruction
// r1 = in context
// r2 = out context
arm_func test_runThumbInstruction
    strh r0, test_runThumbInstruction_instruction
    adr r12, old_context
    stmia r12, {r0-r14}
    ldmia r1, {r0-r14}
    ldr pc,= test_runThumbInstruction_instruction

.thumb
.type test_runThumbInstruction_instruction, %function
test_runThumbInstruction_instruction:
    nop
    nop
    bx pc

.arm
    str r0, new_context
    ldr r0, (old_context + 8)
    stmib r0, {r1-r14}
    ldr r1, new_context
    str r1, [r0]
    adr r12, old_context
    ldmia r12, {r0-r14}
    bx lr

.end
