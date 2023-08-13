.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func emu_regDispStatLoad16
    ldr r11,= emu_ioRegisters
    ldrh r9, [r8]
    ldrh r10, [r11, #4]
    tst r8, #1
    and r9, r9, #7
    bic r10, r10, #0xC7
    orr r9, r9, r10

    movne r9, r9, ror #8

    bx lr

arm_func emu_regDispStatVCountLoad32
    ldr r11,= emu_ioRegisters
    ldrh r9, [r8]
    ldrh r10, [r11, #4]
    and r9, r9, #7
    bic r10, r10, #0xC7
    orr r9, r9, r10

    ldrh r12, [r8, #2]
    cmp r12, #160
        blt 1f
    cmp r12, #192
        movlt r12, #159
        blt 1f
    sub r12, r12, #32
    cmp r12, #227
        movgt r12, #227
1:
    orr r9, r9, r12, lsl #16

    bx lr

arm_func emu_regDispStatStore16
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    ldr r11,= emu_ioRegisters
    ldr r12,= vm_hwIrqMask
    strh r9, [r11, #4]
    ldr r11, [r12]
    and r10, r9, #(7 << 3)
    bic r11, r11, #3
    orr r11, r11, r10, lsr #3
    str r11, [r12]
    ldr r10, [r12, #(vm_forcedIrqMask - vm_hwIrqMask)]
    orr r10, r10, r11
    and r10, r10, #7
    mov r10, r10, lsl #3
    mov r9, r9, lsr #8 // vcount match line
    cmp r9, #160
        addgt r9, r9, #32
    orr r9, r10, r9, lsl #8
    tst r9, #0x10000
        orrne r9, r9, #0x80
    strh r9, [r8]
    bx lr
