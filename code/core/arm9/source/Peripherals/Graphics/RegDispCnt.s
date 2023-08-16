.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func emu_regDispCntStore16
    ldr r11,= emu_ioRegisters
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    strh r9, [r11]
    bic r10, r9, #0x007F
    tst r9, #0x40 // obj mapping mode
        orrne r10, r10, #0x10
    tst r9, #0x20 // hblank free
        orrne r10, r10, #(1 << 23)
    and r11, r9, #0x7
    cmp r11, #1
        orreq r10, r10, #2
        biceq r10, r10, #(1 << 11) // disable bg3
    cmp r11, #2
        orreq r10, r10, #2
        biceq r10, r10, #(3 << 8) // disable bg0 and bg1
    // todo: bitmap modes
    orr r10, r10, #0x10000
    str r10, [r8]
    bx lr
