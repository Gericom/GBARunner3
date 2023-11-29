.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrRd rd
    .balign 8
    arm_func memu_thumbLdrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4

    arm_func memu_thumbLdrRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD32_TABLE_OFFSET] // memu_load32Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load32Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrRd, 8

.macro memu_thumbLdrhRd rd
    .balign 8
    arm_func memu_thumbLdrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5

    arm_func memu_thumbLdrhRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD16_TABLE_OFFSET] // memu_load16Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load16Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhRd, 8

.macro memu_thumbLdrshRegRd rd
    .balign 8
    arm_func memu_thumbLdrshRegR\rd
        tst r8, #1
            bne memu_thumbLdrsbRegR\rd\()_afterAddressComputed

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD16_TABLE_OFFSET] // memu_load16Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load16Undefined
        blx r10

        mov r9, r9, lsl #16
        mov r\rd, r9, asr #16
        memu_thumbReturn
.endm

generate memu_thumbLdrshRegRd, 8

.macro memu_thumbLdrbRd rd
    .balign 8
    arm_func memu_thumbLdrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6

    arm_func memu_thumbLdrbRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD8_TABLE_OFFSET] // memu_load8Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load8Undefined
        blx r10

        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbRd, 8

.macro memu_thumbLdrsbRegRd rd
    .balign 8
    arm_func memu_thumbLdrsbRegR\rd
    memu_thumbLdrsbRegR\rd\()_afterAddressComputed:
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD8_TABLE_OFFSET] // memu_load8Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load8Undefined
        blx r10
        
        mov r9, r9, lsl #24
        mov r\rd, r9, asr #24
        memu_thumbReturn
.endm

generate memu_thumbLdrsbRegRd, 8

.end
