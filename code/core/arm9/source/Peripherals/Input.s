.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regKeyInputLoad16
    ldrh r9, [r8]
    bx lr
