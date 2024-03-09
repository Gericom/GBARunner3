.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "GbaIoRegOffsets.h"
#include "SdCache/SdCacheDefs.h"
#include "DtcmStackDefs.inc"
#include "MemoryEmulator/RomDefs.h"

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

.global memu_itcmLoad8Table
memu_itcmLoad8Table:
    .word memu_load8Bios // 00
    .word memu_load8Undefined // 01
    .word memu_load8Ewram // 02
    .word memu_load8Iwram // 03
    .word memu_load8Io // 04
    .word memu_load8Pltt // 05
    .word memu_load8Vram012 // 06
    .word memu_load8Oam // 07
    .word memu_load8Rom // 08
    .word memu_load8Rom // 09
    .word memu_load8RomHi // 0A
    .word memu_load8RomHi // 0B
    .word memu_load8RomHi // 0C
    .word memu_load8RomHi // 0D
    .word memu_load8Sram // 0E
    .word memu_load8Sram // 0F

arm_func memu_load8Bios
    cmp r8, #0x4000
        bhs memu_load8Undefined
    b memu_load32BiosContinue

arm_func memu_load8Ewram
    cmp r8, #ROM_LINEAR_END_DS_ADDRESS
        addhs r9, r8, #(ROM_LINEAR_GBA_ADDRESS - ROM_LINEAR_DS_ADDRESS)
        bhs memu_load8RomHiContinue

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
    ldr r10,= gShadowPalette
    mov r9, r8, lsl #22
    ldrb r9, [r10, r9, lsr #22]
    bx lr

arm_func memu_load8Vram012
    bic r11, r8, #0xFE0000
    tst r11, #0x10000
        addne r11, r11, #0x3F0000
    ldrb r9, [r11]
    bx lr

arm_func memu_load8Vram345
    bic r11, r8, #0xFE0000
    sub r10, r11, #0x06000000
    cmp r10, #0x14000
        addhs r11, r11, #0x3F0000
    ldrb r9, [r11]
    bx lr

arm_func memu_load8Oam
    bic r9, r8, #0x400
    ldrb r9, [r9]
    bx lr

arm_func memu_load8RomCacheMiss
    ldr r11,= dtcmStackEnd
    // check if we already had a stack
    sub r10, r11, r13
    cmp r10, #(DTCM_STACK_SIZE + DTCM_IRQ_STACK_SIZE)
    mov r10, r13
    // if not begin at the end of the stack
    movhs sp, r11
    push {r0-r3,lr}
    mov r0, r12
    bl sdc_loadRomBlockDirect
    ldrb r9, [r0, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
    pop {r0-r3,lr}
    mov r13, r10
    bx lr

arm_func memu_load8RomHi
    bic r9, r8, #0x06000000
memu_load8RomHiContinue:
    ldr r11,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    bic r12, r9, #(3 << (SDC_BLOCK_SHIFT - 2))
    b memu_load8RomContinue

arm_func memu_load8Sram
    ldr r10,= gSaveData
    mov r11, r8, lsl #17
    ldrb r9, [r10, r11, lsr #17]
    bx lr
