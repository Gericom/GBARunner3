.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

/// @brief Stores a 16-bit value to the given 16-bit aligned GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to store to. The address must be 16-bit aligned.
///           This register is preserved.
/// @param r9 The value to store. Must be 16 bit masked: 0x0000XXXX. Trashed.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
arm_func memu_store16
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    bx lr

.global memu_itcmStore16Table
memu_itcmStore16Table:
    .word memu_store16Bios // 00
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

arm_func memu_store16Undefined
    bx lr

arm_func memu_store16Bios
    cmp r8, #0x4000
        bxhs lr
    bx lr

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
    ldr r11,= memu_store16IoTable
    sub r10, r8, #0x04000000
    ldrh r11, [r11, r10]
    cmp r10, #0x20C
        bxlo r11
    bx lr

arm_func memu_store16Pltt
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    strh r9, [r10]
    bx lr

arm_func memu_store16Vram012
    mov r11, #0x06000000
    movs r10, r8, lsl #15
        addmi r11, r11, #0x3F0000
        bicmi r10, r10, #(0x8000 << 15)

    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bx lr

arm_func memu_store16Vram345
    mov r11, #0x06000000
    movs r10, r8, lsl #15
        bicmi r10, r10, #(0x8000 << 15)

    cmp r10, #(0x14000 << 15)
        addge r11, r11, #0x3F0000
    add r11, r11, r10, lsr #15
    strh r9, [r11]
    bx lr

arm_func memu_store16Oam
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    strh r9, [r10]
    bx lr

arm_func memu_store16Rom
    bx lr

arm_func memu_store16Sram
    bx lr