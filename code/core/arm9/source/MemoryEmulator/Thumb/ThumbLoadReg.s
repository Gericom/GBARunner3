.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrRegRd rd
    arm_func memu_thumbLdrRegR\rd
        add r8, r10, r11
        bl memu_load32
        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrRegRd, 8

.macro memu_thumbLdrhRegRd rd
    arm_func memu_thumbLdrhRegR\rd
        add r8, r10, r11
        bl memu_load16
        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhRegRd, 8

.macro memu_thumbLdrshRegRd rd
    arm_func memu_thumbLdrshRegR\rd
        add r8, r10, r11
        tst r8, #1
            bne memu_thumbLdrsbRegR\rd\()_afterAddressComputed
        bl memu_load16
        mov r9, r9, lsl #16
        mov r\rd, r9, asr #16
        memu_thumbReturn
.endm

generate memu_thumbLdrshRegRd, 8

.macro memu_thumbLdrbRegRd rd
    arm_func memu_thumbLdrbRegR\rd
        add r8, r10, r11
        bl memu_load8
        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbRegRd, 8

.macro memu_thumbLdrsbRegRd rd
    arm_func memu_thumbLdrsbRegR\rd
        add r8, r10, r11
    memu_thumbLdrsbRegR\rd\()_afterAddressComputed:
        bl memu_load8
        mov r9, r9, lsl #24
        mov r\rd, r9, asr #24
        memu_thumbReturn
.endm

generate memu_thumbLdrsbRegRd, 8

.end
