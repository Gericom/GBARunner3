.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func emu_regBgCnt01Store16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    bic r9, r9, #0x2000
    strh r9, [r11, r10]
    bic r9, r9, #0x30
    strh r9, [r8]
    bx lr

arm_func emu_regBgCnt01Store32
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    bic r9, r9, #0x2000
    bic r9, r9, #0x20000000
    str r9, [r11, r10]
    bic r9, r9, #0x30
    bic r9, r9, #0x300000
    str r9, [r8]
    bx lr

arm_func emu_regBgCnt2Store16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    strh r9, [r11, r10]
emu_regBgCnt2Store16Continue:
    ldr r10, [r11]
    bic r9, r9, #0x30
    and r11, r10, #7
    cmp r11, #3
    blt 1f
    ldr r12,= 0x5080 // mode 3 and 5: 0x5084, mode 4: 0x5080
    cmp r11, #4
        orrne r12, r12, #4
        andhs r11, r10, #(1 << 4)
        addhs r12, r12, r11, lsl #6 // mode 4 and 5
    and r9, r9, #3
    orr r9, r12, r9
1:
    strh r9, [r8]
    bx lr

arm_func emu_regBgCnt3Store16
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    strh r9, [r11, r10]
    bic r9, r9, #0x30
    strh r9, [r8]
    bx lr

arm_func emu_regBgCnt23Store32
    ldr r11,= emu_ioRegisters
    and r10, r8, #0xFF
    str r9, [r11, r10]
    mov r10, r9, lsr #16
    bic r10, r10, #0x30
    strh r10, [r8, #2]
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16
    b emu_regBgCnt2Store16Continue
