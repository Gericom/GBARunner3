.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedAddPcRm rm
    arm_func jit_thumbUndefinedAddPcR\rm
        .if \rm < 8
            add r8, r11, #4
            add r8, r8, r\rm
        .elseif \rm < 15
            stmdb sp, {r\rm}^
            nop
            ldr r8, [sp, #-4]
            add lr, r11, #4
            add r8, r8, lr
        .else
            // pc
            add r8, r11, #4
            add r8, r8, r8
        .endif
        orr r8, r8, #1
        b jit_thumbEnsureJitted
.endm

generate jit_thumbUndefinedAddPcRm, 16
