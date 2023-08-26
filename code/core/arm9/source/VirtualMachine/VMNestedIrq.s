.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func vm_enableNestedIrqs
    ldr r0,= (vm_nestedIrqHandler - 0x20)
    mov r0, r0, lsr #2
    orr r0, r0, #0xEA000000
    mov r1, #0x18
    str r0, [r1]
    mrs r0, cpsr
    bic r0, r0, #0x80
    msr cpsr_c, r0
    bx lr

arm_func vm_disableNestedIrqs
    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0
    ldr r0,= (vm_irq - 0x20)
    mov r0, r0, lsr #2
    orr r0, r0, #0xEA000000
    mov r1, #0x18
    str r0, [r1]
    bx lr