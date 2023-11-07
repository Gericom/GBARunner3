.text
.altmacro

#include "AsmMacros.inc"

nestedIrqLevel:
    .word 0

arm_func vm_enableNestedIrqs
    ldr r0, nestedIrqLevel
    cmp r0, #0
    add r0, r0, #1
    str r0, nestedIrqLevel
        bxne lr

    ldr r0,= vm_irqReturnForNestedIrq
    mov r1, #0xE2 // always condition for subs pc, r13, #4
    strb r1, [r0, #3]
    mrs r0, cpsr
    bic r0, r0, #0x80
    msr cpsr_c, r0
    bx lr

arm_func vm_disableNestedIrqs
    ldr r0, nestedIrqLevel
    subs r0, r0, #1
    str r0, nestedIrqLevel
        bxne lr

    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0
    ldr r0,= vm_irqReturnForNestedIrq
    mov r1, #0x92 // LS condition for sublss pc, r13, #4
    strb r1, [r0, #3]
    push {r10,r11,lr}
    bl emu_updateIrqs
    pop {r10,r11,pc}