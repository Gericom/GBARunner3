.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

// not supported yet: C flag for RRX #1

.macro memu_armLoadStoreRlo u
    arm_func memu_armLoadStoreRlo_\u
        bic r9, lr, #0xF000
        orr r9, r9, #0x9000
        strh r9, 1f
        .if \u == 0
            mov r12, #0
        .else
            nop
        .endif
        nop
        nop
        1:
        .if \u == 0
            sub r9, r12, r0, lsl #1 // u=0 -> negate offset
        .else
            mov r9, r0, lsl #1
        .endif
        bx r8
.endm

memu_armLoadStoreRlo 0
memu_armLoadStoreRlo 1

.macro memu_armLoadStoreRm rm, u
    arm_func memu_armLoadStoreR\rm\()_\u
        .if \rm < 15
            and r9, lr, #0x0FE0
            orr r9, r9, #0x9000
            orr r9, r9, #9
            strh r9, 1f
            stmdb r13, {r\rm}^
            nop
            ldr r9, [r13, #-4]
            .if \u == 0
                mov r12, #0
            .else
                // interlock
            .endif
        .else
            // pc is not allowed
            bx r8
            .exitm
        .endif
        1:
        .if \u == 0
            sub r9, r12, r0, lsl #1 // u=0 -> negate offset
        .else
            mov r9, r0, lsl #1
        .endif
        bx r8
.endm

.macro memu_armLoadStoreRm_0 rm
    memu_armLoadStoreRm \rm, 0
.endm

generate memu_armLoadStoreRm_0, 16, 8

.macro memu_armLoadStoreRm_1 rm
    memu_armLoadStoreRm \rm, 1
.endm

generate memu_armLoadStoreRm_1, 16, 8

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadStoreRmTable_0
memu_armLoadStoreRmTable_0:
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreRlo_0
    .word memu_armLoadStoreR8_0
    .word memu_armLoadStoreR9_0
    .word memu_armLoadStoreR10_0
    .word memu_armLoadStoreR11_0
    .word memu_armLoadStoreR12_0
    .word memu_armLoadStoreR13_0
    .word memu_armLoadStoreR14_0
    .word memu_armLoadStoreR15_0

 .global memu_armLoadStoreRmTable_1
memu_armLoadStoreRmTable_1:
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreRlo_1
    .word memu_armLoadStoreR8_1
    .word memu_armLoadStoreR9_1
    .word memu_armLoadStoreR10_1
    .word memu_armLoadStoreR11_1
    .word memu_armLoadStoreR12_1
    .word memu_armLoadStoreR13_1
    .word memu_armLoadStoreR14_1
    .word memu_armLoadStoreR15_1

.end
