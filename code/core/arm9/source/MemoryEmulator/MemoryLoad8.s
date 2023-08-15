.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"

// r8: address, preserved
// r9: returns loaded value; value needs masking of bottom 8 bits
// r13: preserved
// lr: return address
arm_func memu_load8
    cmp r8, #0x10000000
        ldrlo pc, [pc, r8, lsr #22]
    b memu_load8Undefined

    .word memu_load8Bios // 00
    .word memu_load8Undefined // 01
    .word memu_load8Ewram // 02
    .word memu_load8Iwram // 03
    .word memu_load8Io // 04
    .word memu_load8Pltt // 05
    .word memu_load8Vram // 06
    .word memu_load8Oam // 07
    .word memu_load8Rom // 08
    .word memu_load8Rom // 09
    .word memu_load8RomHi // 0A
    .word memu_load8RomHi // 0B
    .word memu_load8RomHi // 0C
    .word memu_load8RomHi // 0D
    .word memu_load8Sram // 0E
    .word memu_load8Sram // 0F

arm_func memu_load8Undefined
    ldr r10,= memu_inst_addr
    msr cpsr_c, #0xD7
    ldr r13,= memu_inst_addr
    str lr, [r13]
    mrs r13, spsr
    movs r13, r13, lsl #27
    msr cpsr_c, #0xD1
    ldr r10, [r10]
    ldrcch r9, [r10] // arm
    ldrcsh r9, [r10, #-4] // thumb
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load8Bios
    cmp r8, #0x4000
        bhs memu_load8Undefined
    bx lr

arm_func memu_load8Ewram
    bic r9, r8, #0x00FC0000
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Iwram
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Io
    ldr r11,= memu_load16IoTable
    sub r9, r8, #0x04000000
    cmp r9, #0x20C
        bhs load8IoHi
    ldrh r11, [r11, r9]
    tst r8, #1
        bne load8IoUnaligned
    bx r11

load8IoHi:
    cmp r9, #0x300
        moveq r9, #0
        bxeq lr
    b memu_load8Undefined

load8IoUnaligned:
    str lr, unalignedReturn
    bic r8, r8, #1
    blx r11
    ldr lr, unalignedReturn
    mov r9, r9, lsr #8
    orr r8, r8, #1
    bx lr

unalignedReturn:
    .word 0

arm_func memu_load8Pltt
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Vram
    ldr r11,= emu_ioRegisters
    bic r10, r8, #0x00FE0000
    ldrh r11, [r11] // GBA REG_DISPCNT
    ldr r12,= 0x06018000
    cmp r10, r12
        bicge r10, #0x8000

    and r12, r11, #7
    cmp r12, #3
        ldrlt r11,= 0x06010000
        ldrge r11,= 0x06014000
    cmp r10, r11
        addge r10, #0x3F0000
    ldrb r9, [r10]
    bx lr

arm_func memu_load8Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Rom
    add r9, r8, #(0x02200000 - 0x08000000)
    ldrb r9, [r9]
    bx lr

arm_func memu_load8RomHi
    bic r9, r8, #0x06000000
    add r9, r8, #(0x02200000 - 0x08000000)
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Sram
    bx lr