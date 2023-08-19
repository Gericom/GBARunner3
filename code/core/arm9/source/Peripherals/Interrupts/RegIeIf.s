.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func emu_regIfLoad16
    ldr r11,= vm_emulatedIfImeIe
    ldr r12, [r11, #(vm_emulatedIrqMask - vm_emulatedIfImeIe)]
    ldr r10, [r11, #(vm_forcedIrqMask - vm_emulatedIfImeIe)]
    ldr r9, [r11, #(vm_hwIEAddr - vm_emulatedIfImeIe)]
    orr r12, r12, r10
    ldrh r9, [r9, #4] // DS REG_IF

    ldr r10, [r11]
    mov r10, r10, lsl #18
    bic r9, r9, r12
    orr r9, r9, r10, lsr #18

    bx lr

arm_func emu_regIfStore16
    ldr r11,= vm_emulatedIfImeIe
    bic r9, r9, #0xC000
    ldr r10, [r11]
    bic r10, r10, r9
    str r10, [r11]

    ldr r12, [r11, #(vm_emulatedIrqMask - vm_emulatedIfImeIe)]
    ldr r10, [r11, #(vm_forcedIrqMask - vm_emulatedIfImeIe)]
    ldr r11, [r11, #(vm_hwIEAddr - vm_emulatedIfImeIe)]
    bic r9, r9, r12
    bic r9, r9, r10
    strh r9, [r11, #4] // ack in DS REG_IF

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
    bics r9, r9, #0xC000
    bic r10, r10, r9
    str r10, [r11]
    ldrne r12, [r11, #(vm_emulatedIrqMask - vm_emulatedIfImeIe)]
    beq 1f // skip ack if there are no bits to ack
    bic r9, r9, r12
    ldr r12, [r11, #(vm_forcedIrqMask - vm_emulatedIfImeIe)]
    ldr r11, [r11, #(vm_hwIEAddr - vm_emulatedIfImeIe)]
    bic r9, r9, r12
    strh r9, [r11, #4] // ack in DS REG_IF
1:
    b emu_updateIrqs
