.altmacro
#include "AsmMacros.inc"

.section ".dtcm", "aw"

emu_lastReadTimerValues:
    .word 0
    .word 0
    .word 0
    .word 0

.section ".itcm", "ax"

arm_func emu_timerCntLLoad16
    ldr r9, [r8]
    ldr r11,= (emu_ioRegisters - 0x04000000)
    tst r9, #(1 << 18) // cascade
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    bxne lr
    ldr r10, [r8, r11]
    mov r9, r9, lsr #1
    movs r10, r10, lsl #16
    beq emu_timerCntLLoad16_reload0
    orrmi r9, r9, #0x8000
    bx lr

emu_timerCntLLoad16_reload0:
    ldr r11,= (emu_lastReadTimerValues - 0x04000100)
    ldrh r12, [r11, r8]
    tst r12, #0x8000
    orrne r9, r9, #0x8000
    cmp r9, r12
    eorlo r9, r9, #0x8000
    strh r9, [r11, r8]
    bx lr

arm_func emu_timerCntLoad32
    ldr r9, [r8]
    ldr r11,= (emu_ioRegisters - 0x04000000)
    tst r9, #(1 << 18) // cascade
    mov r10, r9, lsl #16
    mov r10, r10, lsr #16
    and r9, r9, #0xFF0000
    bne emu_timerCntLoad32_continue
    ldr r11, [r8, r11]
    mov r10, r10, lsr #1
    movs r11, r11, lsl #16
    beq emu_timerCntLLoad32_reload0
    orrmi r10, r10, #0x8000
emu_timerCntLoad32_continue:
    orr r9, r9, r10
    bx lr

emu_timerCntLLoad32_reload0:
    ldr r11,= (emu_lastReadTimerValues - 0x04000100)
    ldrh r12, [r11, r8]
    tst r12, #0x8000
    orrne r10, r10, #0x8000
    cmp r10, r12
    eorlo r10, r10, #0x8000
    strh r10, [r11, r8]
    orr r9, r9, r10
    bx lr

arm_func emu_timerCntLStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    strh r9, [r11, r8]!
    ldrb r11, [r11, #2]
    tst r11, #(1 << 2) // cascade
    movne r12, r9
    moveq r12, r9, lsl #1
    strh r12, [r8]

    tst r8, #8
        bxne lr // timer 2 or 3

    tst r8, #4 // check if timer 1
    mov r11, #0x04000000
    mov r10, #0x30
    orr r10, r10, r9, lsl #9
    orrne r10, r10, #(1 << 8) // timer 1
    str r10, [r11, #0x188]
    bx lr

arm_func emu_timer0CntHStore16
    bic r9, r9, #(1 << 2) // timer 0 has no cascade bit
arm_func emu_timerCntHStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    and r9, r9, #0xC7 // mask out the unused bits
    strh r9, [r11, r8]!

    ldrh r12, [r8]
    bic r12, r9, r12
    tst r12, #0x80
    beq 1f

    ldr r10,= (emu_lastReadTimerValues - 0x04000102)
    ldrh r12, [r11, #-2]
    tst r9, #(1 << 2) // cascade
    strh r12, [r10, r8]
    moveq r12, r12, lsl #1
    strh r12, [r8, #-2]

1:
    strh r9, [r8]

    and r10, r8, #0xC
    mov r10, r10, lsr #2
    mov r12, #8
    mov r10, r12, lsl r10
    ldr r12,= vm_hwIrqMask
    ldr r11, [r12]
    tst r9, #0x40
    orrne r11, r11, r10
    biceq r11, r11, r10
    str r11, [r12]

    tst r10, #0x18 // timer 0 and 1 mask
        bxeq lr // return when timer 2 or 3

    tst r10, #0x10 // check if timer 1
    mov r11, #0x04000000
    mov r10, #0x50
    orr r10, r10, r9, lsl #9
    orrne r10, r10, #(1 << 8) // timer 1
    str r10, [r11, #0x188]
    bx lr

arm_func emu_timer0CntStore32
    bic r9, r9, #(1 << 18) // timer 0 has no cascade bit
arm_func emu_timerCntStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    // mask out the unused bits
    bic r9, r9, #0xFF000000
    bic r9, r9, #0x00380000
    ldr r12, [r11, r8]
    cmp r12, r9
        bxeq lr // nothing changed
    str r9, [r11, r8]

    bic r12, r9, r12
    tst r12, #0x800000
    ldrne r10,= (emu_lastReadTimerValues - 0x04000100)
    strneh r9, [r10, r8]

    tst r9, #(1 << 18) // cascade
    movne r12, r9, lsl #16
    moveq r12, r9, lsl #17
    and r11, r9, #0xFF0000
    orr r12, r11, r12, lsr #16
    str r12, [r8]

    and r10, r8, #0xC
    mov r10, r10, lsr #2
    mov r12, #8
    mov r10, r12, lsl r10
    ldr r12,= vm_hwIrqMask
    ldr r11, [r12]
    tst r9, #0x400000
    orrne r11, r11, r10
    biceq r11, r11, r10
    str r11, [r12]

    tst r10, #0x18 // timer 0 and 1 mask
        bxeq lr // return when timer 2 or 3

    tst r10, #0x10 // check if timer 1
    mov r12, r9, lsl #16
    mov r11, #0x04000000
    mov r10, #0x30
    orr r10, r10, r12, lsr #7
    orrne r10, r10, #(1 << 8) // timer 1
    str r10, [r11, #0x188]
    mov r12, r9, lsr #16
    mov r10, #0x50
    orr r10, r10, r12, lsl #9
    orrne r10, r10, #(1 << 8) // timer 1
    str r10, [r11, #0x188]
    bx lr
