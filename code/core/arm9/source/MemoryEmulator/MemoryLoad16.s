.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"

// r8: address, preserved; unaligned correction is applied
// r9: returns loaded value, if aligned 0x0000XXYY, otherwise 0xYY0000XX
// r13: preserved
// lr: return address
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
    .word memu_load16RomHi // 0A
    .word memu_load16RomHi // 0B
    .word memu_load16RomHi // 0C
    .word memu_load16RomHi // 0D
    .word memu_load16Sram // 0E
    .word memu_load16Sram // 0F

arm_func memu_load16Undefined
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

arm_func memu_load16Bios
    cmp r8, #0x4000
        bhs memu_load16Undefined
    bx lr

arm_func memu_load16Ewram
    bic r9, r8, #0x00FC0000
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Iwram
    ldrh r9, [r8]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Io
    ldr r12,= memu_load16IoTable
    sub r9, r8, #0x04000000
    cmp r9, #0x20C
        bhs memu_load16Undefined
    ldrh r12, [r12, r9]
    tst r8, #1
        bne load16IoUnaligned
    bx r12

load16IoUnaligned:
    str lr, unalignedReturn
    bic r8, r8, #1
    blx r12
    ldr lr, unalignedReturn
    mov r9, r9, ror #8
    orr r8, r8, #1
    bx lr

unalignedReturn:
    .word 0

arm_func memu_load16Pltt
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Vram
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
    ldrh r9, [r10]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Rom
    add r9, r8, #(0x02200000 - 0x08000000)
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16RomHi
    bic r9, r8, #0x06000000
    add r9, r9, #(0x02200000 - 0x08000000)
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Sram
    bx lr