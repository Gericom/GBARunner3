.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regVCountLoad16
    ldrh r9, [r8]
    cmp r9, #160
        bxlt lr
    cmp r9, #192
        movlt r9, #159
        bxlt lr
    sub r9, r9, #32
    cmp r9, #227
        movgt r9, #227
    bx lr

