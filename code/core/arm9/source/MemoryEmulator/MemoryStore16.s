.section ".itcm", "ax"

#include "AsmMacros.inc"

arm_func memu_store16
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    bx lr

    .word memu_store16Bios // 00
    .word memu_store16Undefined // 01
    .word memu_store16Ewram // 02
    .word memu_store16Iwram // 03
    .word memu_store16Io // 04
    .word memu_store16Pltt // 05
    .word memu_store16Vram // 06
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
    strh r9, [r8]
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

arm_func memu_store16Vram
    ldr r11,= emu_ioRegisters
    bic r10, r8, #0x00FE0000
    ldrh r11, [r11]
    ldr r12,= 0x06018000
    cmp r10, r12
        bicge r10, #0x8000

    and r12, r11, #7
    cmp r12, #3
        ldrlt r11,= 0x06010000
        ldrge r11,= 0x06014000
    cmp r10, r11
        addge r10, #0x3F0000
    strh r9, [r10]
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