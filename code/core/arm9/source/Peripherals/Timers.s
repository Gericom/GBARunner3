.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_timerCntLLoad16
    ldrh r9, [r8]
    tst r8, #1
        movne r9, r9, ror #8
    bx lr

arm_func emu_timerCntLoad32
    ldr r9, [r8]
    bx lr
