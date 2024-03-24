.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_dma012CntLStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bics r9, r9, #0xC000
        orreq r9, r9, #0x4000
    strh r9, [r11, r8]
    bx lr

arm_func emu_dmaInternalMemoryAddressHiStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bic r9, r9, #0xF800
    strh r9, [r11, r8]
    bx lr

arm_func emu_dmaAnyMemoryAddressHiStore16
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bic r9, r9, #0xF000
    strh r9, [r11, r8]
    bx lr

arm_func emu_dmaInternalMemoryAddressStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bic r9, r9, #0xF8000000
    str r9, [r11, r8]
    bx lr

arm_func emu_dmaAnyMemoryAddressStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bic r9, r9, #0xF0000000
    str r9, [r11, r8]
    bx lr

arm_func emu_dma012CntStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    bic r9, r9, #0xC000
    movs r12, r9, lsl #16
        orreq r9, r9, #0x4000
    strh r9, [r11, r8]
    mov r9, r9, lsr #16
    b emu_dmaCntHStore16

arm_func emu_dma3CntStore32
    ldr r11,= (emu_ioRegisters - 0x04000000)
    strh r9, [r11, r8]
    mov r9, r9, lsr #16

arm_func emu_dmaCntHStore16
    bic r9, r9, #0x1F
    ldr r11,= (emu_ioRegisters - 8)
    push {r0-r3,lr}
    and r0, r8, #0xFC
    cmp r0, #GBA_REG_OFFS_DMA3CNT
        bicne r9, r9, #(1 << 11) // clear rom dreq bit for channels 0-2
    add r0, r0, r11
    mov r1, r9
    bl dma_CntHStore16
    pop {r0-r3,lr}
    b emu_updateIrqs

// called from C code
// r0 = src
// r1 = dst
// r2 = byte count
// r3 = src step (in halfwords)
arm_func dma_immTransferSafe16
    push {r4,r5,r8-r11,lr}
    ldr r4, [sp, #(4 * 7)] // dst step (in halfwords)
    ldr r5,= dma_transferRegister
1:
    bic r8, r0, #1
    bl memu_load16
    strh r9, [r5]
    strh r9, [r5, #2]
    add r0, r0, r3, lsl #1
    bic r8, r1, #1
    bl memu_store16
    add r1, r1, r4, lsl #1
    subs r2, r2, #2
    bne 1b
    pop {r4,r5,r8-r11,pc}

// called from C code
// r0 = src
// r1 = dst
// r2 = byte count
// r3 = src step (in words)
arm_func dma_immTransferSafe32
    push {r4,r5,r8-r11,lr}
    ldr r4, [sp, #(4 * 7)] // dst step (in words)
    ldr r5,= dma_transferRegister
1:
    bic r8, r0, #3
    bl memu_load32
    str r9, [r5]
    add r0, r0, r3, lsl #2
    bic r8, r1, #3
    bl memu_store32
    add r1, r1, r4, lsl #2
    subs r2, r2, #4
    bne 1b
    pop {r4,r5,r8-r11,pc}

.text

// called from C code
// r0 = dst
// r1 = byte count
// r2 = dst step (in halfwords)
arm_func dma_immTransferSafe16BadSrc
    push {r8-r11,lr}
    ldr r3,= dma_transferRegister
    bic r8, r0, #1
1:
    ldrh r9, [r3]
    bl memu_store16
    add r8, r8, r2, lsl #1
    subs r1, r1, #2
    bne 1b
    pop {r8-r11,pc}

// called from C code
// r0 = dst
// r1 = byte count
// r2 = dst step (in words)
arm_func dma_immTransferSafe32BadSrc
    push {r8-r11,lr}
    ldr r3,= dma_transferRegister
    bic r8, r0, #3
1:
    ldr r9, [r3]
    bl memu_store32
    add r8, r8, r2, lsl #2
    subs r1, r1, #4
    bne 1b
    pop {r8-r11,pc}
