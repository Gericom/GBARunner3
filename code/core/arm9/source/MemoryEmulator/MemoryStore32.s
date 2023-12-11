.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func memu_store32FromC
    push {r8-r11,lr}
    mov r8, r0
    mov r9, r1
    bl memu_store32
    pop {r8-r11,pc}

/// @brief Stores a 32-bit value to the given GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to store to. This register is preserved,
///           except for IO writes which will clear the bottom 2 bits.
/// @param r9 The value to store. Trashed.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
arm_func memu_store32
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
arm_func memu_store32Undefined
    bx lr

.global memu_itcmStore32Table
memu_itcmStore32Table:
    .word memu_store32Undefined // 00
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
    bic r8, r8, #3
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
    bic r10, r9, #0x80000000
    add r11, r11, r10, lsr #15
    ldrh r11, [r11]
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    bic r10, r10, #3
    strh r12, [r10]
    ldr r12,= (gShadowPalette - 0x05000000)
    strh r11, [r10, #2]
    str r9, [r12, r10]
    bx lr

arm_func memu_store32Vram012
    bic r11, r8, #0xFE0000
    tst r11, #0x10000
        addne r11, r11, #0x3F0000
    str r9, [r11]
    bx lr

arm_func memu_store32Vram3
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
    str r9, [r11, r10, lsr #15]!
    bxhs lr

    ldr r12,= 4370
    mov r10, r10, lsr #20
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    orr r9, r9, #0x8000
    orr r9, r9, #0x80000000
    str r9, [r11, r12, lsl #5]
    bx lr

arm_func memu_store32Vram4
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
    str r9, [r11, r10, lsr #15]!
    bxhs lr

    ldr r12,= 4370
    cmp r10, #(0xA000 << 15)
        subhs r10, r10, #(0xA000 << 15)
        addhs r11, r11, #0x6000

    mov r10, r10, lsr #19
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    str r9, [r11, r12, lsl #4]
    bx lr

arm_func memu_store32Vram5
    mov r11, #0x06000000
    mov r10, r8, lsl #15
    cmp r10, #(0x14000 << 15)
        addhs r11, r11, #0x3F0000
    str r9, [r11, r10, lsr #15]!
    bxhs lr

    ldr r12,= 6554
    cmp r10, #(0xA000 << 15)
        subhs r10, r10, #(0xA000 << 15)
        addhs r11, r11, #0x6000

    mov r10, r10, lsr #20
    smulwb r12, r10, r12
    add r11, r11, #0x40000
    orr r9, r9, #0x8000
    orr r9, r9, #0x80000000
    add r12, r12, r12, lsl #1
    str r9, [r11, r12, lsl #6]
    bx lr

arm_func memu_store32Oam
    bic r10, r8, #0x400
    str r9, [r10]
    bx lr

arm_func memu_store32Rom
    bx lr

arm_func memu_store32Sram
    tst r8, #3
        ldreq r10,= gSaveData
        moveq r11, r8, lsl #17
        streqb r9, [r10, r11, lsr #17]!
        streqb r9, [r10, #1]
        streqb r9, [r10, #2]
        streqb r9, [r10, #3]
    bx lr
