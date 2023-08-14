.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_timerCntLLoad16
    ldrh r9, [r8]
    mov r9, r9, lsl #16
    mov r9, r9, asr #1
    mov r9, r9, lsr #16
    bx lr

arm_func emu_timerCntLoad32
    ldr r9, [r8]
    mov r10, r9, lsl #16
    mov r10, r10, asr #1
    and r9, r9, #0xFF0000
    orr r10, r9, r10, lsr #16
    bx lr
