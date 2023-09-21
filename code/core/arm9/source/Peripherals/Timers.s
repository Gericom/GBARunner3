.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_timerCntLLoad16
    ldrh r9, [r8]
    mov r9, r9, lsl #16
    mov r9, r9, asr #1
    mov r9, r9, lsr #16
    bx lr

arm_func emu_timerCntLoad32
    ldr r9, [r8]
    mov r10, r9, lsl #16
    mov r10, r10, asr #1
    and r9, r9, #0xFF0000
    orr r10, r9, r10, lsr #16
    bx lr

arm_func emu_timerCntL01Store16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    cmp r10, #0x100
    strh r9, [r11, r10]
    mov r11, #0x04000000
    mov r10, #0x30
    orr r10, r10, r9, lsl #10
    orrne r10, r10, #(1 << 9) // timer 1
    str r10, [r11, #0x188]
    bx lr

arm_func emu_timerCntH01Store16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    sub r10, r10, #2
    cmp r10, #0x100
    mov r11, #0x04000000
    mov r10, #0x50
    orr r10, r10, r9, lsl #10
    orrne r10, r10, #(1 << 9) // timer 1
    str r10, [r11, #0x188]
    bx lr

arm_func emu_timerCnt01Store32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    ldr r12, [r11, r10]
    cmp r12, r9
        bxeq lr
    str r9, [r11, r10]
    cmp r10, #0x100
    mov r12, r9, lsl #16
    mov r11, #0x04000000
    mov r10, #0x30
    orr r10, r10, r12, lsr #6
    orrne r10, r10, #(1 << 9) // timer 1
    str r10, [r11, #0x188]
    mov r12, r9, lsr #16
    mov r10, #0x50
    orr r10, r10, r12, lsl #10
    orrne r10, r10, #(1 << 9) // timer 1
    str r10, [r11, #0x188]
    bx lr
