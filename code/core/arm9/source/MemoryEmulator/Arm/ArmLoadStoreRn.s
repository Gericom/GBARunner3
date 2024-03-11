.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro memu_armLoadStoreRn rn, p, w
    arm_func memu_armLoadStoreR\rn\()_\p\w
        .if \rn < 8
            .if (\p == 1) && (\w == 0)
                // lo reg, pre, no writeback
                add r8, r9, r\rn // add the offset
                // we assume here that the Z flag is already cleared
                // which will cause Rd to be loaded
            .elseif (\p == 1) && (\w == 1)
                // lo reg, pre, writeback
                add r8, r\rn, r9 // add the offset
                mov r9, r\rn // save old value of Rn in case of aliasing
                mov r\rn, r8 // writeback
                cmp r10, #(\rn << 2)
            .elseif \p == 0
                // lo reg, post
                mov r8, r\rn
                add r\rn, r\rn, r9 // writeback
                mov r9, r8 // save old value of Rn in case of aliasing
                cmp r10, #(\rn << 2)
            .endif
        .elseif \rn < 15
            stmdb r13, {r\rn}^
            nop
            ldr r8, [r13, #-4]
            cmp r10, #(\rn << 2)
            .if (\p == 1) && (\w == 0)
                // hi reg, pre, no writeback
                add r8, r8, r9 // add the offset
                sub r9, r8, r9
            .elseif (\p == 1) && (\w == 1)
                // hi reg, pre, writeback
                add r8, r8, r9 // add the offset
                sub r9, r8, r9
                str r8, [r13, #-4]
                ldmdb r13, {r\rn}^
                nop
            .elseif \p == 0
                // hi reg, post
                add r9, r8, r9
                str r9, [r13, #-4]
                ldmdb r13, {r\rn}^
                nop
                mov r9, r8 // save old value of Rn in case of aliasing
            .endif
        .else
            // rn = pc; writeback is not allowed
            mov r8, #0
            ldr r8, [r8, #memu_inst_addr]
            // interlock
            add r8, r8, r9 // add the offset
            // we assume here that the Z flag is already cleared
            // which will cause Rd to be loaded
        .endif
        ldrb r10, [r11, r8, lsr #24]
        mov pc, r11, lsr #16
.endm

.macro memu_armLoadStoreRn_pw rn
    memu_armLoadStoreRn rn, 0, 0
    memu_armLoadStoreRn rn, 1, 0
    memu_armLoadStoreRn rn, 1, 1
.endm

generate memu_armLoadStoreRn_pw, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadStoreRnTable_00
memu_armLoadStoreRnTable_00:
.global memu_armLoadStoreRnTable_01
memu_armLoadStoreRnTable_01:
    .word memu_armLoadStoreR0_00
    .word memu_armLoadStoreR1_00
    .word memu_armLoadStoreR2_00
    .word memu_armLoadStoreR3_00
    .word memu_armLoadStoreR4_00
    .word memu_armLoadStoreR5_00
    .word memu_armLoadStoreR6_00
    .word memu_armLoadStoreR7_00
    .word memu_armLoadStoreR8_00
    .word memu_armLoadStoreR9_00
    .word memu_armLoadStoreR10_00
    .word memu_armLoadStoreR11_00
    .word memu_armLoadStoreR12_00
    .word memu_armLoadStoreR13_00
    .word memu_armLoadStoreR14_00
    .word memu_armLoadStoreR15_00

.global memu_armLoadStoreRnTable_10
memu_armLoadStoreRnTable_10:
    .word memu_armLoadStoreR0_10
    .word memu_armLoadStoreR1_10
    .word memu_armLoadStoreR2_10
    .word memu_armLoadStoreR3_10
    .word memu_armLoadStoreR4_10
    .word memu_armLoadStoreR5_10
    .word memu_armLoadStoreR6_10
    .word memu_armLoadStoreR7_10
    .word memu_armLoadStoreR8_10
    .word memu_armLoadStoreR9_10
    .word memu_armLoadStoreR10_10
    .word memu_armLoadStoreR11_10
    .word memu_armLoadStoreR12_10
    .word memu_armLoadStoreR13_10
    .word memu_armLoadStoreR14_10
    .word memu_armLoadStoreR15_10

.global memu_armLoadStoreRnTable_11
memu_armLoadStoreRnTable_11:
    .word memu_armLoadStoreR0_11
    .word memu_armLoadStoreR1_11
    .word memu_armLoadStoreR2_11
    .word memu_armLoadStoreR3_11
    .word memu_armLoadStoreR4_11
    .word memu_armLoadStoreR5_11
    .word memu_armLoadStoreR6_11
    .word memu_armLoadStoreR7_11
    .word memu_armLoadStoreR8_11
    .word memu_armLoadStoreR9_11
    .word memu_armLoadStoreR10_11
    .word memu_armLoadStoreR11_11
    .word memu_armLoadStoreR12_11
    .word memu_armLoadStoreR13_11
    .word memu_armLoadStoreR14_11
    .word memu_armLoadStoreR15_11

.end
