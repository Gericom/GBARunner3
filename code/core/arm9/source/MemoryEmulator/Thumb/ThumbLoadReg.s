.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrRegRd rd
    arm_func memu_thumbLdrRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD32_TABLE_OFFSET] // memu_load32Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load32Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrRegRd, 8

.macro memu_thumbLdrhRegRd rd
    arm_func memu_thumbLdrhRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD16_TABLE_OFFSET] // memu_load16Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load16Undefined
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhRegRd, 8

.macro memu_thumbLdrshRegRd rd
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

.macro memu_thumbLdrbRegRd rd
    arm_func memu_thumbLdrbRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_LOAD8_TABLE_OFFSET] // memu_load8Table
        cmp r8, #0x10000000
            ldrhs r10,= memu_load8Undefined
        blx r10

        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbRegRd, 8

.macro memu_thumbLdrsbRegRd rd
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
