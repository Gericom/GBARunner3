.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

/// @brief Handles 16-bit stores to GBA REG_DISPCNT and updates DS REG_DISPCNT accordingly.
/// @param r0-r7 Preserved
/// @param r8 Address; should be 0x04000000. The bottom 2 bits are ignored.
/// @param r9 Value; the top 16 bits are ignored. Trashed.
/// @param r10-r12 Trashed
/// @param r13 Preserved
/// @param lr Return address.
arm_func emu_regDispCntStore16
    ldr r11,= emu_ioRegisters
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    ldrh r12, [r11]
    strh r9, [r11]
    bic r10, r9, #0x007F
    tst r9, #0x40 // obj mapping mode
        orrne r10, r10, #0x10
    tst r9, #0x20 // hblank free
        orrne r10, r10, #(1 << 23)
    and r11, r9, #0x7
    cmp r11, #1
        orreq r10, r10, #2
        biceq r10, r10, #(1 << 11) // disable bg3
    cmp r11, #2
        orreq r10, r10, #2
        biceq r10, r10, #(3 << 8) // disable bg0 and bg1

    // todo: bitmap modes
    orr r10, r10, #0x10000
    str r10, [r8]

    and r12, r12, #7
    sub r12, r12, #3
    sub r11, r11, #3
    teq r11, r12
        bxpl lr

    movs r11, r11
    adrmi r9, bitmapToTiledTable
    adrpl r9, tiledToBitmapTable

tiledBitmapSwitch:
    ldmia r9!, {r10, r11, r12}
    str r11, [r10], #0x40
    str r11, [r12]
    ldmia r9!, {r11, r12}
    str r11, [r10], #0x40
    str r11, [r12]
    ldmia r9!, {r11, r12}
    str r11, [r10], #0x40
    str r11, [r12]
    ldmia r9!, {r11, r12}
    str r11, [r10], #0x40
    str r11, [r12]
    ldmia r9!, {r11, r12}
    str r11, [r10], #0x40
    str r11, [r12]
    ldmia r9!, {r11, r12}
    str r11, [r10]
    str r11, [r12]
    bx lr

bitmapToTiledTable:
    .word memu_load32Table + (6 * 4)
    .word memu_load32Vram012
    .word memu_itcmLoad32Table + (6 * 4)
    .word memu_load16Vram012
    .word memu_itcmLoad16Table + (6 * 4)
    .word memu_load8Vram012
    .word memu_itcmLoad8Table + (6 * 4)
    .word memu_store32Vram012
    .word memu_itcmStore32Table + (6 * 4)
    .word memu_store16Vram012
    .word memu_itcmStore16Table + (6 * 4)
    .word memu_store8Vram012
    .word memu_itcmStore8Table + (6 * 4)

tiledToBitmapTable:
    .word memu_load32Table + (6 * 4)
    .word memu_load32Vram345
    .word memu_itcmLoad32Table + (6 * 4)
    .word memu_load16Vram345
    .word memu_itcmLoad16Table + (6 * 4)
    .word memu_load8Vram345
    .word memu_itcmLoad8Table + (6 * 4)
    .word memu_store32Vram345
    .word memu_itcmStore32Table + (6 * 4)
    .word memu_store16Vram345
    .word memu_itcmStore16Table + (6 * 4)
    .word memu_store8Vram345
    .word memu_itcmStore8Table + (6 * 4)
