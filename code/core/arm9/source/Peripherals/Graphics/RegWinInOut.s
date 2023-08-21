.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_regWinInOutStore16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    bic r9, r9, #0x00C0
    bic r9, r9, #0xC000
    strh r9, [r11, r10]
    strh r9, [r8]
    bx lr

arm_func emu_regWinInOutStore32
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0x000000C0
    bic r9, r9, #0x0000C000
    bic r9, r9, #0x00C00000
    bic r9, r9, #0xC0000000
    str r9, [r11, #GBA_REG_OFFS_WININ]
    str r9, [r8]
    bx lr
