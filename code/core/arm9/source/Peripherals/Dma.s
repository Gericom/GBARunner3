.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_dma012CntLStore16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    bics r9, r9, #0xC000
        orreq r9, r9, #0x4000
    strh r9, [r11, r10]
    bx lr

arm_func emu_dmaStore16
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    bx lr

arm_func emu_dmaStore32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    str r9, [r11, r10]
    bx lr

arm_func emu_dma012CntStore32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    bic r9, r9, #0xC000
    movs r12, r9, lsl #16
        orreq r9, r9, #0x4000
    strh r9, [r11, r10]
    mov r9, r9, lsr #16
    b emu_dmaCntHStore16

arm_func emu_dma3CntStore32
    ldr r11,= emu_ioRegisters
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    mov r9, r9, lsr #16

arm_func emu_dmaCntHStore16
    ldr r11,= (emu_ioRegisters + 2)
    bic r12, r8, #3
    sub r12, r12, #0x04000000
    ldrh r10, [r11, r12]
    tst r9, #0x8000
    beq dmaStop
    bic r9, r9, #0x8000
    strh r9, [r11, r12]!
    tst r10, #0x8000
        bxne lr
    tst r9, #0x3800
        bxne lr
    tst r9, #0x1E0
        bxne lr
    adr r12, dmaTmp
    stmia r12, {r0-r2,r8,lr}
    ldrh r2, [r11, #-2] // count
    ldr r0, [r11, #-10] // src
    ldr r1, [r11, #-6] // dst
    cmp r2, #0
        moveq r2, #0x10000
    tst r9, #0x400
        bne dmaImm32
    
dmaImm16:
    bic r8, r0, #1
    bl memu_load16
    add r0, r0, #2
    bic r8, r1, #1
    bl memu_store16
    add r1, r1, #2
    subs r2, r2, #1
    bne dmaImm16

    adr r12, dmaTmp
    ldmia r12, {r0-r2,r8,lr}
    bx lr

dmaImm32:
    bic r8, r0, #3
    bl memu_load32
    add r0, r0, #4
    bic r8, r1, #3
    bl memu_store32
    add r1, r1, #4
    subs r2, r2, #1
    bne dmaImm32

    adr r12, dmaTmp
    ldmia r12, {r0-r2,r8,lr}
    bx lr

dmaStop:
    strh r9, [r11, r12]
    bx lr

dmaTmp:
    .word 0 // r0
    .word 0 // r1
    .word 0 // r2
    .word 0 // r8
    .word 0 // lr