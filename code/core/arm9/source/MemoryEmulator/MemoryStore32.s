.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

/// @brief Stores a 32-bit value to the given 32-bit aligned GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to store to. The address must be 32-bit aligned.
///           This register is preserved.
/// @param r9 The value to store. Trashed.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
arm_func memu_store32
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    bx lr

.global memu_itcmStore32Table
memu_itcmStore32Table:
    .word memu_store32Bios // 00
    .word memu_store32Undefined // 01
    .word memu_store32Ewram // 02
    .word memu_store32Iwram // 03
    .word memu_store32Io // 04
    .word memu_store32Pltt // 05
    .word memu_store32Vram012 // 06
    .word memu_store32Oam // 07
    .word memu_store32Rom // 08
    .word memu_store32Rom // 09
    .word memu_store32Rom // 0A
    .word memu_store32Rom // 0B
    .word memu_store32Rom // 0C
    .word memu_store32Rom // 0D
    .word memu_store32Sram // 0E
    .word memu_store32Sram // 0F

arm_func memu_store32Undefined
    bx lr

arm_func memu_store32Bios
    cmp r8, #0x4000
        bxhs lr
    bx lr

arm_func memu_store32Ewram
    bic r10, r8, #0x00FC0000
    str r9, [r10]
    bx lr

arm_func memu_store32Iwram
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x00008000
    str r9, [r10]
    bx lr

arm_func memu_store32Io
    ldr r11,= memu_store32IoTable
    sub r10, r8, #0x04000000
    mov r12, r10, lsr #1
    ldrh r11, [r11, r12]
    cmp r10, #0x20C
        bxlo r11
    bx lr

arm_func memu_store32Pltt
    ldr r11,= gColorLut
    mov r12, r9, lsl #17
    add r12, r11, r12, lsr #16
    ldrh r12, [r12]
    bic r9, r9, #0x80000000
    add r11, r11, r9, lsr #15
    ldrh r11, [r11]
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    strh r12, [r10]
    strh r11, [r10, #2]
    bx lr

arm_func memu_store32Vram012
    mov r11, #0x06000000
    movs r10, r8, lsl #15
        addmi r11, r11, #0x3F0000
        bicmi r10, r10, #(0x8000 << 15)

    str r9, [r11, r10, lsr #15]
    bx lr

arm_func memu_store32Vram345
    mov r11, #0x06000000
    movs r10, r8, lsl #15
        bicmi r10, r10, #(0x8000 << 15)

    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
    str r9, [r11, r10, lsr #15]
    bx lr

arm_func memu_store32Oam
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    str r9, [r10]
    bx lr

arm_func memu_store32Rom
    bx lr

arm_func memu_store32Sram
    // todo: unaligned store should be ignored
    ldr r10,= gSaveData
    mov r11, r8, lsl #16
    strb r9, [r10, r11, lsr #16]!
    strb r9, [r10, #1]
    strb r9, [r10, #2]
    strb r9, [r10, #3]
    bx lr
