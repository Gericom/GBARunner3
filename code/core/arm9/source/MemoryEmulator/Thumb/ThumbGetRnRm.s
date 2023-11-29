.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_thumbGetRnRm rn, rm
    arm_func memu_thumbGetR\rn\()R\rm
        add r8, r\rn, r\rm
        bic pc, r12, #7
.endm

.macro make_memu_thumbGetRnRm arg
    memu_thumbGetRnRm %((\arg>>0)&7),%((\arg>>3)&7)
.endm

generate make_memu_thumbGetRnRm, 64
