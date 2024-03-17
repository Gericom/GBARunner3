.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedBCond cond
    arm_func jit_thumbUndefinedBCond\cond
        msr cpsr_f, r10
        .if \cond == 0
            beq jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 1
            bne jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 2
            bcs jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 3
            bcc jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 4
            bmi jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 5
            bpl jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 6
            bvs jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 7
            bvc jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 8
            bhi jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 9
            bls jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 10
            bge jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 11
            blt jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 12
            bgt jit_thumbUndefinedBCondCommonPass
        .elseif \cond == 13
            ble jit_thumbUndefinedBCondCommonPass
        .endif        
        b jit_thumbUndefinedBCondCommonNoPass
.endm

generate jit_thumbUndefinedBCond, 14

arm_func jit_thumbUndefinedBCondCommonPass
    push {r0-r3}
    mov r0, r11 // instruction address
    mov r1, lr  // instruction
    mov r2, #1
#ifndef GBAR3_TEST
    bl jit_handleThumbBCond
#endif
    mov r8, r0
    pop {r0-r3}
#ifndef GBAR3_TEST
    b jit_thumbEnsureJitted
#endif

arm_func jit_thumbUndefinedBCondCommonNoPass
    push {r0-r3}
    mov r0, r11 // instruction address
    mov r1, lr  // instruction
    mov r2, #0
#ifndef GBAR3_TEST
    bl jit_handleThumbBCond
#endif
    mov r8, r0
    pop {r0-r3}
#ifndef GBAR3_TEST
    b jit_thumbEnsureJitted
#endif
