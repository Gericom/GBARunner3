.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_thumbGetRnRm rn, rm
    thumb_func memu_thumbGetR\rn\()R\rm
        mov r10, r\rn
        mov r11, r\rm
        bx r8
.endm

.macro makeAll_memu_thumbGetRnRm arg=0
    memu_thumbGetRnRm %((\arg>>0)&7),%((\arg>>3)&7)
.if \arg<0x3F
    makeAll_memu_thumbGetRnRm %(\arg+1)
.endif
.endm

makeAll_memu_thumbGetRnRm

.end
