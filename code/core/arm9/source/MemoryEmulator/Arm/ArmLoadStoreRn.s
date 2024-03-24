.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

.macro memu_armLoadRn rn, p, w
    arm_func memu_armLoadR\rn\()_\p\w
        .if \rn < 8
            .if (\p == 1) && (\w == 0)
                // lo reg, pre, no writeback
                add r8, r\rn, r9 // add the offset
                ldrb r9, [r12, r8, lsr #24]
                ldr lr, [r10, -r11, lsl #6] // lr = Rd handler address
                // interlock
                ldr r10, [r9, lr]
                mov lr, lr, lsr #16
                bx r10
            .elseif (\p == 1) && (\w == 1)
                // lo reg, pre, writeback
                add r8, r\rn, r9 // add the offset
                ldrb r9, [r12, r8, lsr #24]
                ldr lr, [r10, -r11, lsl #6] // lr = Rd handler address
                mov r\rn, r8 // writeback
                ldr r10, [r9, lr]
                mov lr, lr, lsr #16
                bx r10
            .elseif \p == 0
                // lo reg, post
                ldrb r12, [r12, r\rn, lsr #24]
                ldr lr, [r10, -r11, lsl #6] // lr = Rd handler address
                mov r8, r\rn
                ldr r10, [r12, lr]
                add r\rn, r\rn, r9 // writeback
                mov lr, lr, lsr #16
                bx r10
            .endif
        .elseif \rn < 15
            stmdb sp, {r\rn}^
            ldr r8, [sp, #-4]
            ldr lr, [r10, -r11, lsl #6] // lr = Rd handler address
            .if (\p == 1) && (\w == 0)
                // hi reg, pre, no writeback
                add r8, r8, r9 // add the offset
                ldrb r10, [r12, r8, lsr #24]
                // interlock x2
                ldr r10, [r10, lr]
                mov lr, lr, lsr #16
                bx r10
            .elseif (\p == 1) && (\w == 1)
                // hi reg, pre, writeback
                add r8, r8, r9 // add the offset
                ldrb r10, [r12, r8, lsr #24]
                str r8, [sp, #-4]
                ldmdb sp, {r\rn}^
                ldr r10, [r10, lr]
                mov lr, lr, lsr #16
                bx r10
            .elseif \p == 0
                // hi reg, post
                ldrb r10, [r12, r8, lsr #24]
                add r9, r8, r9
                str r9, [sp, #-4]
                ldr r10, [r10, lr]
                ldmdb sp, {r\rn}^
                mov lr, lr, lsr #16
                bx r10
            .endif
        .endif
.endm

.macro memu_armLoadRn_pw rn
    memu_armLoadRn rn, 0, 0
    memu_armLoadRn rn, 1, 0
    memu_armLoadRn rn, 1, 1
.endm

generate memu_armLoadRn_pw, 15

arm_func memu_armLoadR15
    // rn = pc; writeback is not allowed
    mov r8, #0
    ldr r8, [r8, #memu_inst_addr]
    ldr lr, [r10, -r11, lsl #6] // lr = Rd handler address
    add r8, r8, r9 // add the offset
    // we assume here that the Z flag is already cleared
    // which will cause Rd to be loaded
    ldrb r10, [r12, r8, lsr #24]
    // interlock x2
    ldr r10, [r10, lr]
    mov lr, lr, lsr #16
    bx r10

.macro memu_armStoreRn rn, p, w
    arm_func memu_armStoreR\rn\()_\p\w
        ldr r11, [r10, -r11, lsl #6] // r11 = Rd handler address
        .if \rn < 8
            .if (\p == 1) && (\w == 0)
                // lo reg, pre, no writeback
                add r8, r\rn, r9 // add the offset
                // we assume here that the Z flag is already cleared
                // which will cause Rd to be loaded
                ldrb r10, [r12, r8, lsr #24]
            .elseif (\p == 1) && (\w == 1)
                // lo reg, pre, writeback
                add r8, r\rn, r9 // add the offset
                cmp r10, #(\rn << 2)
                ldrb r10, [r12, r8, lsr #24]
                mov r9, r\rn // save old value of Rn in case of aliasing
                mov r\rn, r8 // writeback
            .elseif \p == 0
                // lo reg, post
                cmp r10, #(\rn << 2)
                ldrb r10, [r12, r\rn, lsr #24]
                mov r8, r\rn
                add r\rn, r\rn, r9 // writeback
                mov r9, r8 // save old value of Rn in case of aliasing
            .endif
        .elseif \rn < 15
            stmdb sp, {r\rn}^
            nop
            ldr r8, [sp, #-4]
            cmp r10, #(\rn << 2)
            .if (\p == 1) && (\w == 0)
                // hi reg, pre, no writeback
                add r8, r8, r9 // add the offset
                ldrb r10, [r12, r8, lsr #24]
                sub r9, r8, r9
            .elseif (\p == 1) && (\w == 1)
                // hi reg, pre, writeback
                add r8, r8, r9 // add the offset
                ldrb r10, [r12, r8, lsr #24]
                sub r9, r8, r9
                str r8, [sp, #-4]
                ldmdb sp, {r\rn}^
                nop
            .elseif \p == 0
                // hi reg, post
                ldrb r10, [r12, r8, lsr #24]
                add r9, r8, r9
                str r9, [sp, #-4]
                ldmdb sp, {r\rn}^
                nop
                mov r9, r8 // save old value of Rn in case of aliasing
            .endif
        .endif
        mov pc, r11, lsr #16
.endm

.macro memu_armStoreRn_pw rn
    memu_armStoreRn rn, 0, 0
    memu_armStoreRn rn, 1, 0
    memu_armStoreRn rn, 1, 1
.endm

generate memu_armStoreRn_pw, 15

arm_func memu_armStoreR15
    // rn = pc; writeback is not allowed
    mov r8, #0
    ldr r8, [r8, #memu_inst_addr]
    ldr r11, [r10, -r11, lsl #6] // r11 = Rd handler address
    add r8, r8, r9 // add the offset
    // we assume here that the Z flag is already cleared
    // which will cause Rd to be loaded
    ldrb r10, [r12, r8, lsr #24]
    mov pc, r11, lsr #16

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadRnTable_00
memu_armLoadRnTable_00:
.global memu_armLoadRnTable_01
memu_armLoadRnTable_01:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR0_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR1_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR2_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR3_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR4_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR5_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR6_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR7_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR8_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR9_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR10_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR11_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR12_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR13_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR14_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR15 + 0x8000)

.global memu_armLoadRnTable_10
memu_armLoadRnTable_10:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR0_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR1_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR2_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR3_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR4_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR5_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR6_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR7_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR8_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR9_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR10_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR11_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR12_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR13_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR14_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR15 + 0x8000)

.global memu_armLoadRnTable_11
memu_armLoadRnTable_11:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR0_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR1_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR2_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR3_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR4_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR5_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR6_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR7_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR8_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR9_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR10_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR11_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR12_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR13_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR14_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armLoadR15 + 0x8000)

.global memu_armStoreRnTable_00
memu_armStoreRnTable_00:
.global memu_armStoreRnTable_01
memu_armStoreRnTable_01:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR0_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR1_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR2_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR3_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR4_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR5_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR6_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR7_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR8_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR9_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR10_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR11_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR12_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR13_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR14_00 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR15 + 0x8000)

.global memu_armStoreRnTable_10
memu_armStoreRnTable_10:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR0_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR1_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR2_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR3_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR4_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR5_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR6_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR7_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR8_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR9_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR10_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR11_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR12_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR13_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR14_10 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR15 + 0x8000)

.global memu_armStoreRnTable_11
memu_armStoreRnTable_11:
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR0_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR1_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR2_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR3_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR4_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR5_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR6_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR7_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR8_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR9_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR10_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR11_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR12_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR13_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR14_11 + 0x8000)
    .short (memu_loadStoreRemapTable & 0xFFFF)
    .short (memu_armStoreR15 + 0x8000)

.end
