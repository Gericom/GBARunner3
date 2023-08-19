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

arm_func emu_dmaInternalMemoryAddressHiStore16
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0xF800
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    bx lr

arm_func emu_dmaAnyMemoryAddressHiStore16
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0xF000
    sub r10, r8, #0x04000000
    strh r9, [r11, r10]
    bx lr

arm_func emu_dmaInternalMemoryAddressStore32
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0xF8000000
    sub r10, r8, #0x04000000
    str r9, [r11, r10]
    bx lr

arm_func emu_dmaAnyMemoryAddressStore32
    ldr r11,= emu_ioRegisters
    bic r9, r9, #0xF0000000
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
    mov r10, r13
    ldr sp,= dtcmStackEnd
    ldr r11,= (emu_ioRegisters - 8)
    push {r0-r3,lr}
    and r0, r8, #0xFC
    add r0, r0, r11
    mov r1, r9
    bl dma_CntHStore16
    pop {r0-r3,lr}
    mov r13, r10
    bx lr

// called from C code
// r0 = src
// r1 = dst
// r2 = count (number of halfwords)
// r3 = src step (in halfwords)
arm_func dma_immTransferSafe16
    push {r8-r12,lr}
1:
    bic r8, r0, #1
    bl memu_load16
    add r0, r0, r3, lsl #1
    bic r8, r1, #1
    bl memu_store16
    add r1, r1, #2
    subs r2, r2, #1
    bne 1b
    pop {r8-r12,pc}

// called from C code
// r0 = src
// r1 = dst
// r2 = count (number of words)
// r3 = src step (in words)
arm_func dma_immTransferSafe32
    push {r8-r12,lr}
1:
    bic r8, r0, #3
    bl memu_load32
    add r0, r0, r3, lsl #2
    bic r8, r1, #3
    bl memu_store32
    add r1, r1, #4
    subs r2, r2, #1
    bne 1b
    pop {r8-r12,pc}
