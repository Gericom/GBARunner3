.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrImmRd rd
    arm_func memu_thumbStrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE32_TABLE_OFFSET] // memu_store32Table
        mov r9, r\rd
        cmp r8, #0x10000000
            blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrImmRd, 8

.macro memu_thumbStrhImmRd rd
    arm_func memu_thumbStrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5

        add r10, r9, r8, lsr #23
        ldrh r10, [r10, #THUMB_STORE16_TABLE_OFFSET] // memu_store16Table
        and r9, r\rd, r9, lsr #16
        cmp r8, #0x10000000
            blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrhImmRd, 8

.macro memu_thumbStrbImmRd rd
    arm_func memu_thumbStrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6

        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE8_TABLE_OFFSET] // memu_store8Table
        and r9, r\rd, #0xFF
        cmp r8, #0x10000000
            blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrbImmRd, 8

.end
