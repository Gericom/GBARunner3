.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_armUndefinedBxRm rm
    arm_func jit_armUndefinedBxR\rm
        ldr r9, [r13, #(vm_undefinedRegTmp - vm_armUndefinedDispatchTable)]! // dummy read, only used to compute an address
        .if \rm < 8
            mov r8, r\rm
        .elseif \rm < 15
            stmia r13, {r\rm}^
            nop
            ldr r8, [r13]
        .else
            // pc
            add r8, r11, #4
        .endif
        ldr r10, [r13, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
        ldr sp,= dtcmStackEnd
        tst r8, #1
            orrne r10, r10, #0x20 // thumb bit
        push {r0-r3}
        mov r0, r8
        bl jit_ensureBlockJitted
        pop {r0-r3}
        msr spsr, r10
        movs pc, r8
.endm

generate jit_armUndefinedBxRm, 16

.section ".dtcm", "aw"

.global jit_armUndefinedBxRmTable
jit_armUndefinedBxRmTable:
    .word jit_armUndefinedBxR0
    .word jit_armUndefinedBxR1
    .word jit_armUndefinedBxR2
    .word jit_armUndefinedBxR3
    .word jit_armUndefinedBxR4
    .word jit_armUndefinedBxR5
    .word jit_armUndefinedBxR6
    .word jit_armUndefinedBxR7
    .word jit_armUndefinedBxR8
    .word jit_armUndefinedBxR9
    .word jit_armUndefinedBxR10
    .word jit_armUndefinedBxR11
    .word jit_armUndefinedBxR12
    .word jit_armUndefinedBxR13
    .word jit_armUndefinedBxR14
    .word jit_armUndefinedBxR15
