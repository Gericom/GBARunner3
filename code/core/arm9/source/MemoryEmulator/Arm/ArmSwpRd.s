.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"

.macro memu_armSwpRd rd
    arm_func memu_armSwpRdR\rd
        .if \rd == 15
            memu_armReturn
            .exitm
        .endif

        push {r9} // value of rm
        bl memu_load32

        .if \rd < 8
            mov r\rd, r9
            pop {r9} // value of rm
        .elseif \rd < 15
            str r9, [sp, #-4]
            ldmdb sp, {r\rd}^
            pop {r9} // value of rm
        .else
            // pc is not allowed
        .endif

        bic r8, r8, #3
        bl memu_store32
        
        memu_armReturn
.endm

generate memu_armSwpRd, 16

.macro memu_armSwpbRd rd
    arm_func memu_armSwpbRdR\rd
        .if \rd == 15
            memu_armReturn
            .exitm
        .endif

        push {r9} // value of rm
        bl memu_load8

        .if \rd < 8
            and r\rd, r9, #0xFF
            ldrb r9, [sp], #4 // value of rm
        .elseif \rd < 15
            and r10, r9, #0xFF
            ldrb r9, [sp], #4 // value of rm
            str r10, [sp, #-4]
            ldmdb sp, {r\rd}^
            @ nop
        .else
            // pc is not allowed
        .endif

        bl memu_store8
        
        memu_armReturn
.endm

generate memu_armSwpbRd, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armSwpRdTable
memu_armSwpRdTable:
    .word memu_armSwpRdR0
    .word memu_armSwpRdR1
    .word memu_armSwpRdR2
    .word memu_armSwpRdR3
    .word memu_armSwpRdR4
    .word memu_armSwpRdR5
    .word memu_armSwpRdR6
    .word memu_armSwpRdR7
    .word memu_armSwpRdR8
    .word memu_armSwpRdR9
    .word memu_armSwpRdR10
    .word memu_armSwpRdR11
    .word memu_armSwpRdR12
    .word memu_armSwpRdR13
    .word memu_armSwpRdR14
    .word memu_armSwpRdR15

.global memu_armSwpbRdTable
memu_armSwpbRdTable:
    .word memu_armSwpbRdR0
    .word memu_armSwpbRdR1
    .word memu_armSwpbRdR2
    .word memu_armSwpbRdR3
    .word memu_armSwpbRdR4
    .word memu_armSwpbRdR5
    .word memu_armSwpbRdR6
    .word memu_armSwpbRdR7
    .word memu_armSwpbRdR8
    .word memu_armSwpbRdR9
    .word memu_armSwpbRdR10
    .word memu_armSwpbRdR11
    .word memu_armSwpbRdR12
    .word memu_armSwpbRdR13
    .word memu_armSwpbRdR14
    .word memu_armSwpbRdR15
