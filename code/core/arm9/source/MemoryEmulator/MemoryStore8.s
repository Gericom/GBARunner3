.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

/// @brief Stores an 8-bit value to the given GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to store to. This register is preserved.
/// @param r9 The value to store. Must be 8-bit masked: 0x000000XX. Trashed.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
arm_func memu_store8
    mov r10, r8, lsr #23
    ldrh r10, [r10, #memu_store8Table]
    cmp r8, #0x10000000
        bxhs lr
    bx r10

arm_func memu_store8Ewram
    bic r10, r8, #0x00FC0000
    strb r9, [r10]
arm_func memu_store8Undefined
    bx lr

arm_func memu_store8Iwram
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x00008000
    strb r9, [r10]
    bx lr

arm_func memu_store8Io
    sub r10, r8, #0x04000000
    sub r11, r10, #0x60
    cmp r11, #0x48
        blo emu_gbaSoundStore8

    ldr r11,= memu_store16IoTable
    ldr r12,= emu_ioRegisters
    ldrh r11, [r11, r10]
    tst r8, #1
    ldrh r12, [r12, r10]
        bne memu_store8IoHi
    cmp r10, #0x20C
        biclo r12, r12, #0xFF
        orrlo r9, r12, r9
        bxlo r11
    // todo: postflag
    bx lr

arm_func memu_store8IoHi
    bic r8, r8, #1
    cmp r10, #0x20C
        biclo r12, r12, #0xFF00
        orrlo r9, r12, r9, lsl #8
        bxlo r11
    mov r11, #0x300
    orr r11, r11, #1
    cmp r10, r11
        beq haltcnt
    bx lr

haltcnt:
    cmp r9, #0
        mcreq p15, 0, r9, c7, c0, 4
    bx lr

arm_func memu_store8Pltt
    orr r9, r9, r9, lsl #8
    b memu_store16Pltt

arm_func memu_store8Vram012
    mov r11, #0x06000000
    movs r10, r8, lsl #15
        bxmi lr

    orr r9, r9, r9, lsl #8
    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bx lr

arm_func memu_store8Vram3
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        bxhs lr

    mov r11, #0x06000000
    orr r9, r9, r9, lsl #8
    b memu_store16Vram3Finish

arm_func memu_store8Vram4
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        bxhs lr

    mov r11, #0x06000000
    orr r9, r9, r9, lsl #8
    b memu_store16Vram4Finish

arm_func memu_store8Vram5
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        bxhs lr

    mov r11, #0x06000000
    orr r9, r9, r9, lsl #8
    b memu_store16Vram5Finish

arm_func memu_store8Rom
    bx lr

arm_func memu_store8Sram
    ldr r10,= gSaveData
    mov r11, r8, lsl #17
    ldrb r11, [r10, r11, lsr #17]!
    and r9, r9, #0xFF
    ldr r12,= gGbaSaveShared
    cmp r9, r11
        bxeq lr
    strb r9, [r10]
    mov r11, #1 // GBA_SAVE_STATE_DIRTY
    strb r11, [r12]
    mov r11, #0
    ldr r12,= emu_vblankIrqSkipSaveCheckInstruction
    mcr p15, 0, r11, c7, c10, 4 // drain write buffer
    str r11, [r12] // nop, do not skip the save check when dirty
    bx lr
