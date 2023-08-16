.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrRegRd rd
    arm_func memu_thumbStrRegR\rd
        bic r8, r8, #3

        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r9, r10, lsr #22
        ldrlo r10, [r9, #0xC1] // memu_store32Table
        mov r9, r\rd
        blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrRegRd, 8

.macro memu_thumbStrhRegRd rd
    arm_func memu_thumbStrhRegR\rd
        bic r8, r8, #1

        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r9, r10, lsr #22
        ldrlo r10, [r9, #0x101] // memu_store16Table

        mov r9, r\rd, lsl #16
        mov r9, r9, lsr #16

        blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrhRegRd, 8

.macro memu_thumbStrbRegRd rd
    arm_func memu_thumbStrbRegR\rd
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r9, r10, lsr #22
        ldrlo r10, [r9, #0x141] // memu_store8Table

        and r9, r\rd, #0xFF

        blxlo r10

        memu_thumbReturn
.endm

generate memu_thumbStrbRegRd, 8

.end
