.text
.altmacro
#include "AsmMacros.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

arm_func VMAluSPCImmTests_SUBS_PC_R0_0
    mov r1, lr
    vmMSR spsr, r0
    adr r0, 1f
    vmSUBS pc, r0, 0
1:
    vmMRS r0, cpsr
    bx r1
