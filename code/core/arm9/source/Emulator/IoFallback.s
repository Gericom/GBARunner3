.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_ioFallbackLoad32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    ldr r9, [r11, r10]
    bx lr

arm_func emu_ioFallbackLoad16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    ldrh r9, [r11, r10]
    bx lr
    
arm_func emu_ioFallbackStore32
    ldr r11,= emu_ioRegisters
    str r9, [r8]
    sub r10, r8, #0x04000000
    str r9, [r11, r10]
    bx lr

arm_func emu_ioFallbackStore16
    ldr r11,= emu_ioRegisters
    strh r9, [r8]
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    bx lr

arm_func emu_ioStore16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    bx lr

arm_func emu_ioStore32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    str r9, [r11, r10]
    bx lr