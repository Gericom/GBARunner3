.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_armSwpRn rn
    arm_func memu_armSwpRnR\rn
        .if \rn < 8
            mov r8, r\rn
        .elseif \rn < 15
            stmdb r13, {r\rn}^
            nop
            ldr r8, [r13, #-4]
        .else
            // pc is not allowed
        .endif
        bx r11
.endm

generate memu_armSwpRn, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armSwpRnTable
memu_armSwpRnTable:
    .word memu_armSwpRnR0
    .word memu_armSwpRnR1
    .word memu_armSwpRnR2
    .word memu_armSwpRnR3
    .word memu_armSwpRnR4
    .word memu_armSwpRnR5
    .word memu_armSwpRnR6
    .word memu_armSwpRnR7
    .word memu_armSwpRnR8
    .word memu_armSwpRnR9
    .word memu_armSwpRnR10
    .word memu_armSwpRnR11
    .word memu_armSwpRnR12
    .word memu_armSwpRnR13
    .word memu_armSwpRnR14
    .word memu_armSwpRnR15
