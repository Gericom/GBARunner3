.text
.altmacro

#include "AsmMacros.inc"

nestedIrqLevel:
    .word 0

arm_func vm_enableNestedIrqs
    mrs r2, cpsr
    and r3, r2, #0x1F
    cmp r3, #0x12
        bxeq lr // do not allow nested irqs when in irq mode

    ldr r0, nestedIrqLevel
    cmp r0, #0
    add r0, r0, #1
    str r0, nestedIrqLevel
        bxne lr

    ldr r0,= vm_irqReturnForNestedIrq
    mov r1, #0xE2 // always condition for subs pc, r13, #4
    strb r1, [r0, #3]
    bic r2, r2, #0x80
    msr cpsr_c, r2
    bx lr

arm_func vm_disableNestedIrqs
    mrs r2, cpsr
    and r3, r2, #0x1F
    cmp r3, #0x12
        bxeq lr // do not allow nested irqs when in irq mode

    ldr r0, nestedIrqLevel
    subs r0, r0, #1
    str r0, nestedIrqLevel
        bxne lr

    orr r2, r2, #0x80
    msr cpsr_c, r2
    ldr r0,= vm_irqReturnForNestedIrq
    mov r1, #0x92 // LS condition for sublss pc, r13, #4
    strb r1, [r0, #3]
    push {r10,r11,lr}
    bl emu_updateIrqs
    pop {r10,r11,pc}