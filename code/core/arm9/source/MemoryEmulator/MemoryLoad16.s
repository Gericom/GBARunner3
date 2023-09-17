.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"
#include "GbaIoRegOffsets.h"
#include "SdCache/SdCacheDefs.h"

/// @brief Loads a 16-bit value from the given GBA memory address.
///        When unaligned rotation is applied.
/// @param r0-r7 Preserved.
/// @param r8 The address to load from. This register is preserved.
/// @param r9 Returns the loaded value. If aligned 0x0000XXYY, otherwise 0xYY0000XX.
/// @param r10-r12 Trashed.
/// @param r13 Preserved.
/// @param lr Return address.
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

arm_func memu_load16UndefinedZero
    mov r9, #0
    bx lr

arm_func memu_load16Undefined
    ldr r10,= memu_inst_addr
    msr cpsr_c, #0xD7
    ldr r13,= memu_inst_addr
    str lr, [r13]
    mrs r13, spsr
    movs r13, r13, lsl #27
    msr cpsr_c, #0xD1
    ldr r10, [r10]
    subcs r10, r10, #4
    cmp r10, #0x08000000
        bhs undefinedFromRom16

    ldrh r9, [r10]
undefined16Continue:
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

undefinedTmpR8:
    .word 0
undefinedTmpLr:
    .word 0

undefinedFromRom16:
    str r8, undefinedTmpR8
    str lr, undefinedTmpLr
    mov r8, r10
    bl memu_load16Rom
    ldr r8, undefinedTmpR8
    ldr lr, undefinedTmpLr
    b undefined16Continue

arm_func memu_load16Bios
    cmp r8, #0x4000
        bhs memu_load16Undefined
    ldr r9,= 0xE3A02004
    tst r8, #2
        moveq r9, r9, lsl #16
    mov r9, r9, lsr #16
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Ewram
    bic r9, r8, #0x00FC0000
    ldrh r9, [r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Iwram
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x00008000
    ldrh r9, [r9]
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
    ldr r11,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    bic r12, r8, #(3 << (SDC_BLOCK_SHIFT - 2))
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
        beq load16RomCacheMiss
    mov r9, r9, lsr #(32 - SDC_BLOCK_SHIFT)
    ldrh r9, [r11, r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

load16RomCacheMiss:
    mov r10, r13
    ldr r11,= dtcmStackEnd
    // check if we already had a stack
    sub r9, r11, r13
    cmp r9, #512
    // if not begin at the end of the stack
    movhs sp, r11
    push {r0-r3,lr}

#ifdef GBAR3_HICODE_CACHE_MAPPING
    bl hic_unmapRomBlock
#endif

    bl ic_invalidateAll
    mov r0, r8
    bl sdc_loadRomBlockDirect
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    mov r9, r9, lsr #(32 - SDC_BLOCK_SHIFT)
    ldrh r9, [r0, r9]
    pop {r0-r3,lr}
    mov r13, r10
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16RomHi
    ldr r11,= sdc_romBlockToCacheBlock
    bic r9, r8, #0x0E000000
    bic r12, r9, #(3 << (SDC_BLOCK_SHIFT - 2))
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
        beq load16RomCacheMiss
    mov r9, r9, lsr #(32 - SDC_BLOCK_SHIFT)
    ldrh r9, [r11, r9]
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr

arm_func memu_load16Sram
    ldr r10,= gSaveData
    mov r11, r8, lsl #16
    ldrb r9, [r10, r11, lsr #16]
    orr r9, r9, lsl #8
    tst r8, #1
        bxeq lr
    mov r9, r9, ror #8
    bx lr
