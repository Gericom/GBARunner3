.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.macro memu_thumbGetRn rn, rm
    arm_func memu_thumbGetR\rn\()_\rm
        mov r10, r\rn
        bic pc, r12, #7
.endm

.macro make_memu_thumbGetRn arg
    memu_thumbGetRn %((\arg>>0)&7),%((\arg>>3)&7)
.endm

generate make_memu_thumbGetRn, 64
