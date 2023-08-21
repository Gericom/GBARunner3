.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_regBldCntStore16
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0xC000
    strh r9, [r11, #GBA_REG_OFFS_BLDCNT]
    strh r9, [r8]
    bx lr

arm_func emu_regBldAlphaStore16
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0x00E0
    bic r9, r9, #0xE000
    strh r9, [r11, #GBA_REG_OFFS_BLDALPHA]
    strh r9, [r8]
    bx lr

arm_func emu_regBldCntBldAlphaStore32
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0x0000C000
    bic r9, r9, #0x00E00000
    bic r9, r9, #0xE0000000
    str r9, [r11, #GBA_REG_OFFS_BLDCNT]
    str r9, [r8]
    bx lr
