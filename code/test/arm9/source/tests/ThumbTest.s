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
test_runThumbInstruction_instruction_done:
    str r0, new_context
    ldr r0, (old_context + 8)
    stmib r0, {r1-r14}
    ldr r1, new_context
    str r1, [r0]
    adr r12, old_context
    ldmia r12, {r0-r14}
    bx lr

// r0 = instruction
// r1 = in context
// r2 = out context
arm_func test_isThumbInstructionUndefined
    mov r3, #0
    str r3, undefined_caught
    adr r12, (undefined_vector - 12)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
    mov r3, #0
	str r12, [r3, #4]
    adr r12, (prefetch_abort_vector - 20)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
	str r12, [r3, #12]
    adr r12, (prefetch_abort_vector - 8)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
	str r12, [r3, #0]
    push {lr}
    bl test_runThumbInstruction
    pop {lr}
    ldr r0, undefined_caught
    bx lr


undefined_caught:
    .word 0

.arm
undefined_vector:
    mov r13, #1
    str r13, undefined_caught
    movs pc, lr

prefetch_abort_vector:
    ldr lr,= test_runThumbInstruction_instruction_done
    movs pc, lr

.end
