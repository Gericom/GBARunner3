.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func jit_armUndefinedB
    sub lr, lr, #0x02000000
    str lr, [r11, #-4]
    mov r10, #0
    mcr p15, 0, r10, c7, c10, 4
    mcr p15, 0, r10, c7, c5, 0
    mov r12, lr, lsl #8
    ldr r10, [r10, #vm_undefinedSpsr]
    add r8, r11, r12, asr #6
    push {r0-r3}
    add r0, r8, #4
    bl jit_ensureBlockJitted
    pop {r0-r3}
    msr spsr, r10
    adds pc, r8, #4

.section ".dtcm", "aw"

.balign 64

.global jit_armUndefinedBTable
jit_armUndefinedBTable:
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
    .word jit_armUndefinedB
