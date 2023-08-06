.section ".itcm", "ax"

#include "AsmMacros.inc"

// r8 = address; should be preserved
// lr = return address
// return value in r9
arm_func memu_load32
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    b memu_load32Undefined

    .word memu_load32Bios // 00
    .word memu_load32Undefined // 01
    .word memu_load32Ewram // 02
    .word memu_load32Iwram // 03
    .word memu_load32Io // 04
    .word memu_load32Pltt // 05
    .word memu_load32Vram // 06
    .word memu_load32Oam // 07
    .word memu_load32Rom // 08
    .word memu_load32Rom // 09
    .word memu_load32Rom // 0A
    .word memu_load32Rom // 0B
    .word memu_load32Rom // 0C
    .word memu_load32Rom // 0D
    .word memu_load32Sram // 0E
    .word memu_load32Sram // 0F

arm_func memu_load32Undefined
    bx lr

arm_func memu_load32Bios
    cmp r8, #0x4000
        bhs memu_load32Undefined
    bx lr

arm_func memu_load32Ewram
    bic r9, r8, #0x00FC0000
    ldr r9, [r9]
    bx lr

arm_func memu_load32Iwram
    ldr r9, [r8]
    bx lr

arm_func memu_load32Io
    ldr r11,= memu_load32IoTable
    sub r9, r8, #0x04000000
    mov r10, r9, lsr #1
    ldrh r11, [r11, r10]
    cmp r9, #0x20C
        bxlo r11
    b memu_load32Undefined

arm_func memu_load32Pltt
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldr r9, [r9]
    bx lr

arm_func memu_load32Vram
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
    ldr r9, [r10]
    bx lr

arm_func memu_load32Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldr r9, [r9]
    bx lr

arm_func memu_load32Rom
    bx lr

arm_func memu_load32Sram
    bx lr