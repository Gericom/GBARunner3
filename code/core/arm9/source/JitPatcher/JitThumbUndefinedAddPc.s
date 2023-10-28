.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedAddPcRm rm
    arm_func jit_thumbUndefinedAddPcR\rm
        .if \rm < 8
            add r8, r\rm, #4
            b jit_thumbUndefinedAddPcRmCommon
        .elseif \rm < 15
            stmdb sp, {r\rm}^
            b jit_thumbUndefinedAddPcRmCommonHiReg
        .else
            // pc
            add r8, r11, #4
            add r8, r8, r8
            orr r8, r8, #1
            b jit_thumbEnsureJitted
        .endif
.endm

generate jit_thumbUndefinedAddPcRm, 16

arm_func jit_thumbUndefinedAddPcRmCommonHiReg
    ldr r8, [sp, #-4]
    add r8, r8, #4

arm_func jit_thumbUndefinedAddPcRmCommon
    add r8, r8, r11
    orr r8, r8, #1
    b jit_thumbEnsureJitted
