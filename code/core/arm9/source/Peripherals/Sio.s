.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_regSioCntSioMultiSendStore32
    ldr r11,= (emu_ioRegisters + GBA_REG_OFFS_SIOMULTI_SEND)
    mov r12, r9, ror #16
    strh r12, [r11]
    mov r9, r12, lsr #16

arm_func emu_regSioCntStore16
    ldr r11,= (emu_ioRegisters + GBA_REG_OFFS_SIOCNT)
    ldrh r10, [r11, #(GBA_REG_OFFS_RCNT - GBA_REG_OFFS_SIOCNT)]
    and r12, r9, #0x3000
    and r10, r10, #0xC000
    orr r10, r10, r12
    mov r10, r10, lsr #12
    cmp r10, #8
        andlo r10, r10, #3
        andhs r10, r10, #0xC
    cmp r10, #2
        bhs regSioCntStore16_multi

    // normal mode
    orr r9, r9, #(1 << 2) // SI
    and r12, r9, #0x81
    cmp r12, #0x81
        bne regSioCntStore16_end
    bic r9, r9, #0x80
    strh r9, [r11]
    tst r9, #0x4000 // irq
        bxeq lr
    ldr r12,= vm_emulatedIfImeIe
    ldr r10, [r12]
    orr r10, r10, #(1 << 7) // SIO IRQ
    str r10, [r12]
    b emu_updateIrqs

regSioCntStore16_multi:
    biceq r9, r9, #0x7C
    orreq r9, r9, #0xC

regSioCntStore16_end:
    strh r9, [r11]
    bx lr
