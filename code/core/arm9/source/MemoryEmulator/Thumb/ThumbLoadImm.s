.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrImmRd rd
    arm_func memu_thumbLdrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD32_TABLE_OFFSET] // memu_load32Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load32Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrImmRd, 8

.macro memu_thumbLdrhImmRd rd
    arm_func memu_thumbLdrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD16_TABLE_OFFSET] // memu_load16Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load16Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhImmRd, 8

.macro memu_thumbLdrbImmRd rd
    arm_func memu_thumbLdrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD8_TABLE_OFFSET] // memu_load8Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load8Undefined
        blx r10

        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbImmRd, 8

.end
