.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedBxRm rm
    arm_func jit_thumbUndefinedBxR\rm
        .if \rm < 8
            mov r8, r\rm
            b jit_thumbEnsureJitted
        .elseif \rm < 15
            stmdb sp, {r\rm}^
            b jit_thumbEnsureJittedHiReg
        .else
            // pc
            add r8, r11, #4
            bic r8, r8, #2 // 32 bit align target address
            b jit_thumbEnsureJitted
        .endif
.endm

generate jit_thumbUndefinedBxRm, 16
