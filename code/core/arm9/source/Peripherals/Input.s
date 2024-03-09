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

arm_func emu_regKeyInputKeyControlStore32
    mov r9, r9, lsr #16
arm_func emu_regKeyControlStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    orr r10, r8, #2
    bic r9, r9, #0x3C00
    strh r9, [r10]
    strh r9, [r11, r10]

    // interrupt
    ldr r12,= vm_hwIrqMask
    ldr r11, [r12]
    tst r9, #(1 << 14)
    biceq r11, r11, #(1 << 12)
    orrne r11, r11, #(1 << 12)
    str r11, [r12]

    bx lr
