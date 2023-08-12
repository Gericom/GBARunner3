.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regDispStatStore16
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    ldr r11,= emu_ioRegisters
    ldr r12,= vm_hwIrqMask
    strh r9, [r11, #4]
    ldr r11, [r12]
    and r9, r9, #(7 << 3)
    bic r11, r11, #3
    orr r11, r11, r9, lsr #3
    str r11, [r12]
	bx lr
