.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbStrImmRd rd
    arm_func memu_thumbStrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4
        bic r8, r8, #3
        mov r9, r\rd
        bl memu_store32
        memu_thumbReturn
.endm

generate memu_thumbStrImmRd, 8

.macro memu_thumbStrhImmRd rd
    arm_func memu_thumbStrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5
        bic r8, r8, #1
        mov r9, r\rd, lsl #16
        mov r9, r9, lsr #16
        bl memu_store16
        memu_thumbReturn
.endm

generate memu_thumbStrhImmRd, 8

.macro memu_thumbStrbImmRd rd
    arm_func memu_thumbStrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6
        and r9, r\rd, #0xFF
        bl memu_store8
        memu_thumbReturn
.endm

generate memu_thumbStrbImmRd, 8

.end
