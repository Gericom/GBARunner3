.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

/// @brief Stores a 16-bit value to the given GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to store to. This register is preserved,
///           except for IO writes which will clear the bottom bit.
/// @param r9 The value to store. Must be 16 bit masked: 0x0000XXXX. Trashed.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
arm_func memu_store16
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
arm_func memu_store16Undefined
    bx lr

.global memu_itcmStore16Table
memu_itcmStore16Table:
    .word memu_store16Undefined // 00
    .word memu_store16Undefined // 01
    .word memu_store16Ewram // 02
    .word memu_store16Iwram // 03
    .word memu_store16Io // 04
    .word memu_store16Pltt // 05
    .word memu_store16Vram012 // 06
    .word memu_store16Oam // 07
    .word memu_store16Rom // 08
    .word memu_store16Rom // 09
    .word memu_store16Rom // 0A
    .word memu_store16Rom // 0B
    .word memu_store16Rom // 0C
    .word memu_store16Rom // 0D
    .word memu_store16Sram // 0E
    .word memu_store16Sram // 0F

arm_func memu_store16Ewram
    bic r10, r8, #0x00FC0000
    strh r9, [r10]
    bx lr

arm_func memu_store16Iwram
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x00008000
    strh r9, [r10]
    bx lr

arm_func memu_store16Io
    bic r8, r8, #1
    ldr r11,= memu_store16IoTable
    sub r10, r8, #0x04000000
    ldrh r11, [r11, r10]
    cmp r10, #0x20C
        bxlo r11
    bx lr

arm_func memu_store16Pltt
    ldr r11,= gColorLut
    bic r12, r9, #0x8000
    add r11, r11, r12, lsl #1
    ldrh r11, [r11]
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    ldr r12,= (gShadowPalette - 0x05000000)
    strh r11, [r10]
    strh r9, [r12, r10]
    bx lr

arm_func memu_store16Vram012
    bic r11, r8, #0xFE0000
    tst r11, #0x10000
        addne r11, r11, #0x3F0000
    strh r9, [r11]
    bx lr

arm_func memu_store16Vram3
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
arm_func memu_store16Vram3Finish
    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bxhs lr

    ldr r12,= 4370
    mov r10, r10, lsr #20
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    orr r9, r9, #0x8000
    mov r12, r12, lsl #5
    strh r9, [r11, r12]
    bx lr

arm_func memu_store16Vram4
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
arm_func memu_store16Vram4Finish
    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bxhs lr

    ldr r12,= 4370
    cmp r10, #(0xA000 << 15)
        subhs r10, r10, #(0xA000 << 15)
        addhs r11, r11, #0x6000

    mov r10, r10, lsr #19
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    mov r12, r12, lsl #4
    strh r9, [r11, r12]
    bx lr

arm_func memu_store16Vram5
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
arm_func memu_store16Vram5Finish
    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bxhs lr

    ldr r12,= 6554
    cmp r10, #(0xA000 << 15)
        subhs r10, r10, #(0xA000 << 15)
        addhs r11, r11, #0x6000

    mov r10, r10, lsr #20
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    orr r9, r9, #0x8000
    add r12, r12, r12, lsl #1
    mov r12, r12, lsl #6
    strh r9, [r11, r12]
    bx lr

arm_func memu_store16Oam
    bic r10, r8, #0x400
    strh r9, [r10]
    bx lr

arm_func memu_store16Rom
    bx lr

arm_func memu_store16Sram
    tst r8, #1
        ldreq r10,= gSaveData
        moveq r11, r8, lsl #17
        streqb r9, [r10, r11, lsr #17]!
        streqb r9, [r10, #1]
    bx lr
