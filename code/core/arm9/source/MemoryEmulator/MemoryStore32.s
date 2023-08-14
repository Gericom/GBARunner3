.section ".itcm", "ax"

#include "AsmMacros.inc"

// r8: address, must be 4-aligned, preserved
// r9: value
// r13: preserved
// lr: return address
arm_func memu_store32
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    bx lr

    .word memu_store32Bios // 00
    .word memu_store32Undefined // 01
    .word memu_store32Ewram // 02
    .word memu_store32Iwram // 03
    .word memu_store32Io // 04
    .word memu_store32Pltt // 05
    .word memu_store32Vram // 06
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
    str r9, [r8]
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
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    str r9, [r10]
    bx lr

arm_func memu_store32Vram
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
    str r9, [r10]
    bx lr

arm_func memu_store32Oam
    bic r10, r8, #0x00FF0000
    bic r10, r10, #0x0000FC00
    str r9, [r10]
    bx lr

arm_func memu_store32Rom
    bx lr

arm_func memu_store32Sram
    bx lr