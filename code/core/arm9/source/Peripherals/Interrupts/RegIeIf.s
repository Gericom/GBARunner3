.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regIeStore16
    ldr r11,= (emu_ioRegisters + 0x200)
    bic r9, r9, #0xC000
    strh r9, [r11]

    ldr r11,= vm_emulatedIfImeIe
    mov r9, r9, lsl #17
    ldr r10, [r11]
    mov r10, r10, lsl #16
    orr r10, r9, r10, lsr #16
    str r10, [r11]

    // todo: if irqs are available now, we may need to trigger an irq in the vm

	bx lr

arm_func emu_regIfLoad16
    ldr r11,= vm_emulatedIfImeIe
    ldr r9, [r11]
    mov r9, r9, lsl #18
    mov r9, r9, lsr #18
    tst r8, #1
        movne r9, r9, ror #8
	bx lr

arm_func emu_regIfStore16
    ldr r11,= vm_emulatedIfImeIe
    bic r9, r9, #0xC000
    ldr r10, [r11]
    bic r10, r10, r9
    str r10, [r11]
	bx lr

arm_func emu_regIeIfLoad32
    ldr r11,= (emu_ioRegisters + 0x200)
    ldrh r9, [r11]

    ldr r11,= vm_emulatedIfImeIe
    ldr r10, [r11]
    mov r10, r10, lsl #18
    orr r9, r9, r10, lsr #2   

    bx lr

arm_func emu_regIeIfStore32
    ldr r11,= (emu_ioRegisters + 0x200)
    bic r9, r9, #0xC000
    strh r9, [r11]

    ldr r11,= vm_emulatedIfImeIe
    mov r12, r9, lsl #17
    ldr r10, [r11]
    mov r10, r10, lsl #16
    orr r10, r12, r10, lsr #16
    mov r9, r9, lsr #16
    bic r9, r9, #0xC000
    bic r10, r10, r9
    str r10, [r11]

    // todo: if irqs are available now, we may need to trigger an irq in the vm

    bx lr
