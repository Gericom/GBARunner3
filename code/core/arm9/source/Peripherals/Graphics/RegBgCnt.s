.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func emu_regBgCnt01Store16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    bic r9, r9, #0x2000
    strh r9, [r11, r10]
    bic r9, r9, #0x30
    strh r9, [r8]
    bx lr

arm_func emu_regBgCnt01Store32
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    bic r9, r9, #0x2000
    bic r9, r9, #0x20000000
    str r9, [r11, r10]
    bic r9, r9, #0x30
    bic r9, r9, #0x300000
    str r9, [r8]
    bx lr

arm_func emu_regBgCnt23Store16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    strh r9, [r11, r10]
    bic r9, r9, #0x30
    strh r9, [r8]
    bx lr

arm_func emu_regBgCnt23Store32
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    str r9, [r11, r10]
    bic r9, r9, #0x30
    bic r9, r9, #0x300000
    str r9, [r8]
    bx lr
