.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regImeStore16
    ldr r11,= emu_ioRegisters
    and r9, r9, #1
    strb r9, [r11, #0x208]

    ldr r11,= vm_emulatedIfImeIe
    ldr r10, [r11]
    bic r10, r10, #(1 << 15)
    orr r10, r10, r9, lsl #15
    str r10, [r11]

    // todo: if irqs were switched on, we may need to trigger an irq in the vm

    bx lr
