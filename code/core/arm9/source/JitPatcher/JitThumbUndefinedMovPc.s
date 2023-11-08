.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedMovPcRm rm
    arm_func jit_thumbUndefinedMovPcR\rm
        .if \rm < 8
            orr r8, r\rm, #1
            b jit_thumbEnsureJitted
        .elseif \rm < 15
            stmdb sp, {r\rm}^
            b jit_thumbUndefinedMovPcRmCommonHiReg
        .else
            // pc
            add r8, r11, #5
            b jit_thumbEnsureJitted
        .endif
.endm

generate jit_thumbUndefinedMovPcRm, 16

arm_func jit_thumbUndefinedMovPcRmCommonHiReg
    ldr r8, [sp, #-4]
    orr r8, r8, #1
    b jit_thumbEnsureJitted

