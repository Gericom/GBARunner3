.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrRegRd rd
    arm_func memu_thumbStrRegR\rd
        bic r8, r8, #3
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE32_TABLE_OFFSET] // memu_store32Table
        mov r9, r\rd
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrRegRd, 8

.macro memu_thumbStrhRegRd rd
    arm_func memu_thumbStrhRegR\rd
        bic r8, r8, #1
        add r10, r9, r8, lsr #23
        ldrh r10, [r10, #THUMB_STORE16_TABLE_OFFSET] // memu_store16Table
        and r9, r\rd, r9, lsr #16
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrhRegRd, 8

.macro memu_thumbStrbRegRd rd
    arm_func memu_thumbStrbRegR\rd
        add r9, r9, r8, lsr #23
        ldrh r10, [r9, #THUMB_STORE8_TABLE_OFFSET] // memu_store8Table
        and r9, r\rd, #0xFF
        cmp r8, #0x10000000
            blxlo r10
        memu_thumbReturn
.endm

generate memu_thumbStrbRegRd, 8

.end
