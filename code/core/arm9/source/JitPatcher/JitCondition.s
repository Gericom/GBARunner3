.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

// r0 = cpsr, r1 = condition
arm_func jit_conditionPass
    mov r1, r1, lsl #4
    strb r1, (1f + 3)
    msr cpsr_f, r0
    mov r0, #1
    b 1f
1:
    sub r0, r0, r0
    rsb r0, r0, #1
    bx lr
