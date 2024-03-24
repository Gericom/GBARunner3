.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_armSwpRn rn
    arm_func memu_armSwpRnR\rn
        ldr r11, [r10, -r11, lsl #6] // r11 = Rd handler address
        .if \rn < 8
            mov r8, r\rn
        .elseif \rn < 15
            stmdb sp, {r\rn}^
            ldr r8, [sp, #-4]
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
    .short 0
    .short memu_armSwpRnR0
    .short 0
    .short memu_armSwpRnR1
    .short 0
    .short memu_armSwpRnR2
    .short 0
    .short memu_armSwpRnR3
    .short 0
    .short memu_armSwpRnR4
    .short 0
    .short memu_armSwpRnR5
    .short 0
    .short memu_armSwpRnR6
    .short 0
    .short memu_armSwpRnR7
    .short 0
    .short memu_armSwpRnR8
    .short 0
    .short memu_armSwpRnR9
    .short 0
    .short memu_armSwpRnR10
    .short 0
    .short memu_armSwpRnR11
    .short 0
    .short memu_armSwpRnR12
    .short 0
    .short memu_armSwpRnR13
    .short 0
    .short memu_armSwpRnR14
    .short 0
    .short memu_armSwpRnR15
