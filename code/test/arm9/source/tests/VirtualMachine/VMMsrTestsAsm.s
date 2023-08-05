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
