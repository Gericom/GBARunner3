.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrRd rd
    .balign 8
    arm_func memu_thumbStrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4

    arm_func memu_thumbStrRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE32_TABLE_OFFSET] // memu_store32Table
        mov r9, r\rd
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrRd, 8

.macro memu_thumbStrhRd rd
    .balign 8
    arm_func memu_thumbStrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5

    arm_func memu_thumbStrhRegR\rd
        add r10, r9, r8, lsr #23
        ldrh r10, [r10, #THUMB_STORE16_TABLE_OFFSET] // memu_store16Table
        and r9, r\rd, r9, lsr #16
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrhRd, 8

.macro memu_thumbStrbRd rd
    .balign 8
    arm_func memu_thumbStrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6

    arm_func memu_thumbStrbRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE8_TABLE_OFFSET] // memu_store8Table
        and r9, r\rd, #0xFF
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrbRd, 8

.end
