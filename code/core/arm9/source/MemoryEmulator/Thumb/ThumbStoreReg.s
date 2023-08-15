.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrRegRd rd
    arm_func memu_thumbStrRegR\rd
        bic r8, r8, #3
        mov r9, r\rd
        bl memu_store32
        memu_thumbReturn
.endm

generate memu_thumbStrRegRd, 8

.macro memu_thumbStrhRegRd rd
    arm_func memu_thumbStrhRegR\rd
        bic r8, r8, #1
        mov r9, r\rd, lsl #16
        mov r9, r9, lsr #16
        bl memu_store16
        memu_thumbReturn
.endm

generate memu_thumbStrhRegRd, 8

.macro memu_thumbStrbRegRd rd
    arm_func memu_thumbStrbRegR\rd
        and r9, r\rd, #0xFF
        bl memu_store8
        memu_thumbReturn
.endm

generate memu_thumbStrbRegRd, 8

.end
