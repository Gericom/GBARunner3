.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"
#include "GbaSioDefinitions.h"

arm_func emu_regSioCntSioMultiSendLoad32
    ldr r11,= (emu_ioRegisters + GBA_REG_OFFS_SIOCNT)
    ldr r10,= gGbaSioShared
    ldr r9, [r11]
    b emu_regSioCntLoad16Continue

arm_func emu_regSioCntLoad16
    ldr r11,= (emu_ioRegisters + GBA_REG_OFFS_SIOCNT)
    ldr r10,= gGbaSioShared
    ldrh r9, [r11]
emu_regSioCntLoad16Continue:
    mov r11, #0
    mcr p15, 0, r11, c7, c10, 4 // drain write buffer
    mcr p15, 0, r10, c7, c6, 1 // invalidate range
    ldrb r12, [r10, #9] // SI
    bic r9, r9, #GBA_SIOCNT_SI
    cmp r12, #0
    orrne r9, r9, #GBA_SIOCNT_SI
    bx lr

arm_func emu_regSioCntSioMultiSendStore32
    ldr r11,= (emu_ioRegisters + GBA_REG_OFFS_SIOMULTI_SEND)
    mov r12, r9, ror #16
    strh r12, [r11]
    mov r9, r12, lsr #16

arm_func emu_regSioCntStore16
    mov r10, r13
    ldr sp,= dtcmStack + 128
    push {r0-r3,r10,lr}
    mov r0, r9
    bl sio_sioCntStore16
    ldmfd sp, {r0-r3,r13,pc}

arm_func emu_regRcntStore32
    mov r9, r9, lsl #16
    mov r9, r9, lsr #16

arm_func emu_regRcntStore16
    mov r10, r13
    ldr sp,= dtcmStack + 128
    push {r0-r3,r10,lr}
    mov r0, r9
    bl sio_rcntStore16
    pop {r0-r3,r13,pc}