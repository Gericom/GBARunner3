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

.text

.macro jit_thumbPlaceBackBxRm rm
    arm_func jit_thumbPlaceBackBxR\rm
        ldr r8,= (0x4700 | ((\rm) << 3))
        b jit_thumbPlaceBackBxRmCommon
.endm

generate jit_thumbPlaceBackBxRm, 16

arm_func jit_thumbPlaceBackBxRmCommon
    strh r8, [r11]
    mov r8, #0
    mcr p15, 0, r8, c7, c10, 4
    mcr p15, 0, r8, c7, c5, 0
    msr spsr, r10
    movs pc, r11
