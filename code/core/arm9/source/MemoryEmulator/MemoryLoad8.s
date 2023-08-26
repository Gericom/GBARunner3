.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"
#include "GbaIoRegOffsets.h"
#include "SdCache/SdCacheDefs.h"

/// @brief Loads an 8-bit value from the given GBA memory address.
/// @param r0-r7 Preserved.
/// @param r8 The address to load from. This register is preserved.
/// @param r9 Returns the loaded value. The value requires masking of the bottom 8 bits.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
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
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x00008000
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
    ldrb r9, [r10]
    bx lr

arm_func memu_load8Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldrb r9, [r9]
    bx lr

arm_func memu_load8Rom
    ldr r11,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    bic r12, r8, #(3 << (SDC_BLOCK_SHIFT - 2))
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
    ldrneb r9, [r11, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
    bxne lr

load8RomCacheMiss:
    mov r10, r13
    ldr r11,= dtcmStackEnd
    // check if we already had a stack
    sub r9, r11, r13
    cmp r9, #512
    // if not begin at the end of the stack
    movhs sp, r11
    push {r0-r3,lr}
    mov r0, r8
    bl sdc_loadRomBlockDirect
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    ldrb r9, [r0, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
    pop {r0-r3,lr}
    mov r13, r10
    bx lr

arm_func memu_load8RomHi
    ldr r11,= sdc_romBlockToCacheBlock
    bic r9, r8, #0x0E000000
    bic r12, r9, #(3 << (SDC_BLOCK_SHIFT - 2))
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
    ldrneb r9, [r11, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
    bxne lr
    b load8RomCacheMiss

arm_func memu_load8Sram
    bx lr