.text
.altmacro
#include "AsmMacros.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

.macro VMMsrTests_SetVMSpsrRm rm
    arm_func VMMsrTests_SetVMSpsrR\rm
        vmMSR spsr, r\rm
        bx lr
.endm

generate VMMsrTests_SetVMSpsrRm, 14

arm_func VMMsrTests_SetVMSpsrFlagsR0
    vmMSR spsr_f, r0
    bx lr

arm_func VMMsrTests_SetVMSpsrControlR0
    vmMSR spsr_c, r0
    bx lr

arm_func VMMsrTests_SetVMCpsrFlagsR0
    vmMSR cpsr_f, r0
    vmMRS r0, cpsr
    bx lr

arm_func VMMsrTests_SetVMCpsrFlagsR0VerifyHwCpsr
    vmMSR cpsr_f, r0
    mrs r0, cpsr
    bx lr

arm_func VMMsrTests_SetVMCpsrControlR0
    mov r1, lr // return address not in a banked register
    msr cpsr_f, #0 // clear flags
    vmMSR cpsr_c, r0
    vmMRS r0, cpsr
    bx r1
