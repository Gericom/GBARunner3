.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"
#include "GbaIoRegOffsets.h"

/// @brief Loads a 32-bit value from the given GBA memory address.
///        When unaligned rotation is applied.
/// @param r0-r7 Preserved.
/// @param r8 The address to load from. This register is preserved.
/// @param r9 Returns the loaded value.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
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
    .word memu_load32RomHi // 0A
    .word memu_load32RomHi // 0B
    .word memu_load32RomHi // 0C
    .word memu_load32RomHi // 0D
    .word memu_load32Sram // 0E
    .word memu_load32Sram // 0F

arm_func memu_load32Undefined
    ldr r10,= memu_inst_addr
    msr cpsr_c, #0xD7
    ldr r13,= memu_inst_addr
    str lr, [r13]
    mrs r13, spsr
    movs r13, r13, lsl #27
    msr cpsr_c, #0xD1
    ldr r10, [r10]
    bcs thumbUndefined32

armUndefined32:
    ldr r10, [r10]
    mov r11, r8, lsl #3
    mov r9, r10, ror r11
    bx lr

thumbUndefined32:
    ldrh r11, [r10, #-4]
    orr r10, r11, r11, lsl #16
    mov r11, r8, lsl #3
    mov r9, r10, ror r11
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
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x00008000
    ldr r9, [r9]
    bx lr

arm_func memu_load32Io
    ldr r11,= memu_load32IoTable
    sub r9, r8, #0x04000000
    cmp r9, #0x20C
        bhs memu_load32Undefined
    mov r10, r9, lsr #1
    ldrh r11, [r11, r10]
    ands r12, r8, #3
        bne load32IoUnaligned
    bx r11

load32IoUnaligned:
    orr lr, lr, r12
    str lr, unalignedReturn
    bic r8, r8, #3
    blx r11
    ldr lr, unalignedReturn
    mov r12, lr, lsl #3
    mov r9, r9, ror r12
    and r12, lr, #3
    orr r8, r8, r12
    bic pc, lr, #3

unalignedReturn:
    .word 0

arm_func memu_load32Pltt
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldr r9, [r9]
    bx lr

arm_func memu_load32Vram
    ldr r11,= emu_ioRegisters
    bic r10, r8, #0x00FE0000
    ldrh r11, [r11, #GBA_REG_OFFS_DISPCNT]
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
    add r9, r8, #(0x02200000 - 0x08000000)
    ldr r9, [r9]
    bx lr

arm_func memu_load32RomHi
    bic r9, r8, #0x06000000
    add r9, r9, #(0x02200000 - 0x08000000)
    ldr r9, [r9]
    bx lr

arm_func memu_load32Sram
    bx lr