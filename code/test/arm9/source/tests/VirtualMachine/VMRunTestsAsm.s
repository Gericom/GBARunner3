.text
#include "AsmMacros.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

arm_func VMRunTests_GetVMCpsr
    vmMRS r0, cpsr
    bx lr
    