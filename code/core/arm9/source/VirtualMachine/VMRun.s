.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

// r0 = start address
// r1 = context
// r2 = store context
arm_func vm_run
    str r2, vm_storedContext
    stmia r2, {r0-lr}
    ldr r12,= (vm_undefined - 12)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
    mov r3, #0
	str r12, [r3, #4]
    ldr r12,= (vm_swi - 16)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
	str r12, [r3, #8]
    ldr r12,= (vm_irq - 0x20)
    mov r12, r12, lsr #2
	orr r12, r12, #0xEA000000
	str r12, [r3, #0x18]
    mov lr, r0
    ldmia r1, {r0-sp}
    blx lr
    ldr r2, vm_storedContext
    ldmib r2, {r1-lr}
    bx lr

vm_storedContext:
    .word 0
