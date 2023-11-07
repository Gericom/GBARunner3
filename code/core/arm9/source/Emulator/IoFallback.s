.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_ioFallbackLoad32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    ldr r9, [r11, r8]
    bx lr

arm_func emu_ioFallbackLoad16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    ldrh r9, [r11, r8]
    bx lr

arm_func emu_ioFallbackStore16
    strh r9, [r8]

arm_func emu_ioStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    strh r9, [r11, r8]
    bx lr

arm_func emu_ioFallbackStore32
    str r9, [r8]

arm_func emu_ioStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    str r9, [r11, r8]
    bx lr