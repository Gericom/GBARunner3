.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regKeyInputLoad16
    ldrh r9, [r8]
    bx lr

arm_func emu_regKeyInputKeyControlLoad32
    ldr r11,= (emu_ioRegisters + 2)
    sub r10, r8, #0x04000000
    ldrh r10, [r11, r10] // key control
    ldrh r9, [r8]
    orr r9, r9, r10, lsl #16
    bx lr
