.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_regImeStore16
    ldr r12,= vm_emulatedIfImeIe
    ldr r11,= emu_ioRegisters
    and r9, r9, #1
    ldr r10, [r12]
    strb r9, [r11, #GBA_REG_OFFS_IME]
    bic r10, r10, #(1 << 15)
    orr r10, r10, r9, lsl #15
    str r10, [r12]
    b emu_updateIrqs
