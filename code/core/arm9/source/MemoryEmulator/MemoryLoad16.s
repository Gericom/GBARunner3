.section ".itcm", "ax"

#include "AsmMacros.inc"

arm_func memu_load16
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    b memu_load16Undefined

    .word memu_load16Bios // 00
    .word memu_load16Undefined // 01
    .word memu_load16Ewram // 02
    .word memu_load16Iwram // 03
    .word memu_load16Io // 04
    .word memu_load16Pltt // 05
    .word memu_load16Vram // 06
    .word memu_load16Oam // 07
    .word memu_load16Rom // 08
    .word memu_load16Rom // 09
    .word memu_load16Rom // 0A
    .word memu_load16Rom // 0B
    .word memu_load16Rom // 0C
    .word memu_load16Rom // 0D
    .word memu_load16Sram // 0E
    .word memu_load16Sram // 0F

arm_func memu_load16Undefined
    bx lr

arm_func memu_load16Bios
    cmp r8, #0x4000
        bhs memu_load16Undefined
    bx lr

arm_func memu_load16Ewram
    bic r9, r8, #0x00FC0000
    ldrh r9, [r9]
    bx lr

arm_func memu_load16Iwram
    ldrh r9, [r8]
    bx lr

arm_func memu_load16Io
    ldr r12,= memu_load16IoTable
    sub r9, r8, #0x04000000
    ldrh r12, [r12, r9]
    cmp r9, #0x20C
        bxlo r12
    b memu_load16Undefined

arm_func memu_load16Pltt
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrh r9, [r9]
    bx lr

arm_func memu_load16Vram
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
    ldrh r9, [r10]
    bx lr

arm_func memu_load16Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrh r9, [r9]
    bx lr

arm_func memu_load16Rom
    bic r9, r8, #0xFE000000
    add r9, r9, #0x02200000
    ldrh r9, [r9]
    bx lr

arm_func memu_load16Sram
    bx lr