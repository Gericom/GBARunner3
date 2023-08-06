.text
.altmacro
#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

arm_func VMIrqTests_IrqsEnabledWithPendingIrqJumpsToIrqVector
    ldr r0,= vm_emulatedIfImeIe
    ldr r1,= 0x28001
    str r1, [r0]
    mov r0, #0
    str r0, irqTriggered
    vmMRS r0, cpsr
    bic r0, r0, #0x80
    vmMSR cpsr_c, r0
    // here the irq should be triggered
    ldr r0, irqTriggered
    bx lr

irqTriggered:
    .word 0

tmpR8:
    .word 0

arm_func VMIrqTests_IrqVector
    mov r13, #1
    str r13, irqTriggered
    str r8, tmpR8
    ldr r8,= vm_emulatedIfImeIe
    mov r13, #0
    str r13, [r8]
    ldr r8, tmpR8
    vmSUBS pc, lr, 4
