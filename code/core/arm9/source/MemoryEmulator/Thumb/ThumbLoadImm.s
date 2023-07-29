.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrImmRd rd
    arm_func memu_thumbLdrImmR\rd
        and r8, lr, #0x7C0
        add r8, r10, r8, lsr #4
        bl memu_load32
        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrImmRd, 8

.macro memu_thumbLdrhImmRd rd
    arm_func memu_thumbLdrhImmR\rd
        and r8, lr, #0x7C0
        add r8, r10, r8, lsr #5
        bl memu_load16
        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhImmRd, 8

.macro memu_thumbLdrbImmRd rd
    arm_func memu_thumbLdrbImmR\rd
        and r8, lr, #0x7C0
        add r8, r10, r8, lsr #6
        bl memu_load8
        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbImmRd, 8

.end
