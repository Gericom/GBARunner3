.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_vblankIrq
    // For center and mask display capture has to be enabled every frame
    // and the buffers need to be swapped
    ldr sp,= dtcmIrqStackEnd
    push {r12,lr}
jumpToCaptureUpdate:
    nop

updateDisplayCaptureVramC:
    ldr lr,= 0x84808036
    mov r12, #0x04000000
    strh lr, [r12, #0x66]! // REG_DISPCAPCNT
    mov lr, lr, lsr #16
    add r12, r12, #(0x242 - 0x66)
    strh lr, [r12]
    ldr r12, jumpToUpdateDisplayCaptureVramDInstruction
    b checkSaveWrite

updateDisplayCaptureVramD:
    ldr lr,= 0x80848037
    mov r12, #0x04000000
    strh lr, [r12, #0x66]! // REG_DISPCAPCNT
    mov lr, lr, lsr #16
    add r12, r12, #(0x242 - 0x66)
    strh lr, [r12], -r12 // r12 becomes 0

checkSaveWrite:
    str r12, jumpToCaptureUpdate
.global emu_vblankIrqSkipSaveCheckInstruction
emu_vblankIrqSkipSaveCheckInstruction:
    pop {r12,pc}
#ifndef GBAR3_TEST
    ldr r12,= gGbaSaveShared
    mcr p15, 0, r12, c7, c6, 1 // invalidate range
    ldrb lr, [r12]
    cmp lr, #3 // GBA_SAVE_STATE_WRITE
    popne {r12,pc}

    push {r0-r3}
    bl sav_writeSaveToFile
    pop {r0-r3,r12,pc}
#endif

jumpToUpdateDisplayCaptureVramDInstruction:
    add pc, pc, #(updateDisplayCaptureVramD - jumpToCaptureUpdate - 8)

.pool
.end
