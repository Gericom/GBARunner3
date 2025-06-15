.text
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

#define IRQ_RETURN_FOR_NESTED_IRQ_ENABLE    0xE2 // always condition for subs pc, r13, #4
#define IRQ_RETURN_FOR_NESTED_IRQ_DISABLE   0x92 // LS condition for sublss pc, r13, #4

.global gIrqYieldingEnabled
gIrqYieldingEnabled:
    .word 0

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
    mov r1, #IRQ_RETURN_FOR_NESTED_IRQ_ENABLE
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
    mov r1, #IRQ_RETURN_FOR_NESTED_IRQ_DISABLE
    strb r1, [r0, #3]
    push {r10,r11,lr}
    bl emu_updateIrqs
    pop {r10,r11,pc}

yieldGbaIrqsGbaMode:
    swi 0x7F0000 // swiVMReturnFromYield

arm_func vm_disableIrqYielding
    mov r1, #0
    adr r2, gIrqYieldingEnabled
    swp r0, r1, [r2]
    bx lr

arm_func vm_restoreIrqYielding
    adr r1, gIrqYieldingEnabled
    str r0, [r1]
    bx lr

.section ".itcm", "ax"

arm_func vm_yieldGbaIrqs
    mrs r0, cpsr
    and r1, r0, #0x1F
    cmp r1, #0x11
        movne r0, #0
        bxne lr // only allow yielding from fiq mode

    ldr r2,= (vm_irqReturnForNestedIrq + 3)
    mov r1, #IRQ_RETURN_FOR_NESTED_IRQ_DISABLE
    swpb r3, r1, [r2]
    ldr r12, [r1, #(memu_inst_addr - IRQ_RETURN_FOR_NESTED_IRQ_DISABLE)]

    msr cpsr_c, #0xD7 // goto abt mode
    mrs r1, spsr
    mov r2, lr
    msr cpsr_c, #0xD1 // back to fiq mode, with irqs off

    push {r0-r12,lr}
    bl emu_updateIrqs
    ldr r0,= yieldGbaIrqsGbaMode
    msr spsr, #0x10
    movs pc, r0

arm_func vm_returnFromYield
    msr cpsr_c, #0xD1 // fiq mode
    ldr lr,= (vm_irqReturnForNestedIrq + 3)
    pop {r0-r12}
    strb r3, [lr], -lr // lr = 0
    str r12, [lr, #memu_inst_addr]
    msr cpsr_c, #0xD7 // goto abt mode
    msr spsr, r1
    mov lr, r2
    msr cpsr, r0
    mov r0, #1
    pop {pc}
