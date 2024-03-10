.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_armLoadStoreShortRm rm, u
    arm_func memu_armLoadStoreShortR\rm\()_\u
        .if \rm < 8
            .if \u == 0
                rsb r9, r\rm, #0
            .else
                mov r9, r\rm
            .endif
        .elseif \rm < 15
            stmdb r13, {r\rm}^
            nop
            ldr r9, [r13, #-4]
            .if \u == 0
                rsb r9, r9, #0
            .endif
        .else
            // pc is not allowed
        .endif
        bx r8
.endm

.macro memu_armLoadStoreShortRm_u rm
    memu_armLoadStoreShortRm \rm, 0
    memu_armLoadStoreShortRm \rm, 1
.endm

generate memu_armLoadStoreShortRm_u, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadStoreShortRmTable_0
memu_armLoadStoreShortRmTable_0:
    .word memu_armLoadStoreShortR0_0
    .word memu_armLoadStoreShortR1_0
    .word memu_armLoadStoreShortR2_0
    .word memu_armLoadStoreShortR3_0
    .word memu_armLoadStoreShortR4_0
    .word memu_armLoadStoreShortR5_0
    .word memu_armLoadStoreShortR6_0
    .word memu_armLoadStoreShortR7_0
    .word memu_armLoadStoreShortR8_0
    .word memu_armLoadStoreShortR9_0
    .word memu_armLoadStoreShortR10_0
    .word memu_armLoadStoreShortR11_0
    .word memu_armLoadStoreShortR12_0
    .word memu_armLoadStoreShortR13_0
    .word memu_armLoadStoreShortR14_0
    .word memu_armLoadStoreShortR15_0

 .global memu_armLoadStoreShortRmTable_1
memu_armLoadStoreShortRmTable_1:
    .word memu_armLoadStoreShortR0_1
    .word memu_armLoadStoreShortR1_1
    .word memu_armLoadStoreShortR2_1
    .word memu_armLoadStoreShortR3_1
    .word memu_armLoadStoreShortR4_1
    .word memu_armLoadStoreShortR5_1
    .word memu_armLoadStoreShortR6_1
    .word memu_armLoadStoreShortR7_1
    .word memu_armLoadStoreShortR8_1
    .word memu_armLoadStoreShortR9_1
    .word memu_armLoadStoreShortR10_1
    .word memu_armLoadStoreShortR11_1
    .word memu_armLoadStoreShortR12_1
    .word memu_armLoadStoreShortR13_1
    .word memu_armLoadStoreShortR14_1
    .word memu_armLoadStoreShortR15_1

.end
