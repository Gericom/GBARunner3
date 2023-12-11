.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "GbaIoRegOffsets.h"
#include "SdCache/SdCacheDefs.h"
#include "DtcmStackDefs.inc"
#include "MemoryEmulator/RomDefs.h"

arm_func memu_load32FromC
    push {r8-r11,lr}
    mov r8, r0
    bl memu_load32
    mov r0, r9
    pop {r8-r11,lr}
    bx lr

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

.global memu_itcmLoad32Table
memu_itcmLoad32Table:
    .word memu_load32Bios // 00
    .word memu_load32Undefined // 01
    .word memu_load32Ewram // 02
    .word memu_load32Iwram // 03
    .word memu_load32Io // 04
    .word memu_load32Pltt // 05
    .word memu_load32Vram012 // 06
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
    cmp r10, #0x08000000
        bhs undefinedFromRom32Arm

    ldr r10, [r10]
armUndefined32Continue:
    mov r11, r8, lsl #3
    mov r9, r10, ror r11
    bx lr

thumbUndefined32:
    sub r10, r10, #4
    cmp r10, #0x08000000
        bhs undefinedFromRom32Thumb
    ldrh r11, [r10]
thumbUndefined32Continue:
    orr r10, r11, r11, lsl #16
    mov r11, r8, lsl #3
    mov r9, r10, ror r11
    bx lr

undefinedTmpR8:
    .word 0
undefinedTmpLr:
    .word 0

undefinedFromRom32Arm:
    str r8, undefinedTmpR8
    str lr, undefinedTmpLr
    mov r8, r10
    bl memu_load32Rom
    mov r10, r9
    ldr r8, undefinedTmpR8
    ldr lr, undefinedTmpLr
    b armUndefined32Continue

undefinedFromRom32Thumb:
    str r8, undefinedTmpR8
    str lr, undefinedTmpLr
    mov r8, r10
    bl memu_load16Rom
    mov r11, r9
    ldr r8, undefinedTmpR8
    ldr lr, undefinedTmpLr
    b thumbUndefined32Continue

arm_func memu_load32Bios
    cmp r8, #0x4000
        bhs memu_load32Undefined
arm_func memu_load32BiosContinue
    ldr r9,= memu_biosOpcodeId
    ldr r10,= memu_biosOpcodes
    ldrb r9, [r9]
    and r12, r8, #3
    add r10, r10, r12
    ldr r9, [r10, r9, lsl #2]
    bx lr

arm_func memu_load32Ewram
    cmp r8, #ROM_LINEAR_END_DS_ADDRESS
        addhs r9, r8, #(ROM_LINEAR_GBA_ADDRESS - ROM_LINEAR_DS_ADDRESS)
        bhs memu_load32RomHiContinue

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
    ldr r10,= gShadowPalette
    mov r9, r8, lsl #22
    ldr r9, [r10, r9, lsr #22]
    bx lr

arm_func memu_load32Vram012
    bic r11, r8, #0xFE0000
    tst r11, #0x10000
        addne r11, r11, #0x3F0000
    ldr r9, [r11]
    bx lr

arm_func memu_load32Vram345
    bic r11, r8, #0xFE0000
    sub r10, r11, #0x06000000
    cmp r10, #0x14000
        addhs r11, r11, #0x3F0000
    ldr r9, [r11]
    bx lr

arm_func memu_load32Oam
    bic r9, r8, #0x00FF0000
    bic r9, r9, #0x0000FC00
    ldr r9, [r9]
    bx lr

arm_func memu_load32RomCacheMiss
    ldr r11,= dtcmStackEnd
    // check if we already had a stack
    sub r10, r11, r13
    cmp r10, #DTCM_STACK_SIZE
    mov r10, r13
    // if not begin at the end of the stack
    movhs sp, r11
    push {r0-r3,lr}
    mov r0, r12
    bl sdc_loadRomBlockDirect
    ldr r9, [r0, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
    pop {r0-r3,lr}
    mov r13, r10
    bx lr

arm_func memu_load32RomHi
    bic r9, r8, #0x0E000000
memu_load32RomHiContinue:
    ldr r11,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    bic r12, r9, #(3 << (SDC_BLOCK_SHIFT - 2))
    b memu_load32RomContinue

arm_func memu_load32Sram
    ldr r10,= gSaveData
    mov r11, r8, lsl #17
    ldrb r9, [r10, r11, lsr #17]
    orr r9, r9, lsl #8
    orr r9, r9, lsl #16
    bx lr
