.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_vblankIrq
    // For center and mask display capture has to be enabled every frame
    // and the buffers need to be swapped
    ldr sp,= dtcmIrqStackEnd
    push {r0-r1}
    mov r0, #0x04000000
    ldr r1, [r0, #0x64]! // REG_DISPCAPCNT
    eor r1, r1, #(1 << 16) // toggle between VRAM C and D
    orr r1, r1, #(1 << 31) // enable capture again
    str r1, [r0], #(0x242 - 0x64)
    tst r1, #(1 << 16)
    moveq r1, #0x480 // LCDC
    movne r1, #0x084 // BG or OBJ
    orr r1, r1, #0x8000
    strh r1, [r0] // REG_VRAMCNT_C and REG_VRAMCNT_D

#ifndef GBAR3_TEST
    ldr r0,= gGbaSaveShared
    mcr p15, 0, r0, c7, c6, 1 // invalidate range
    ldrb r1, [r0]
    cmp r1, #3 // GBA_SAVE_STATE_WRITE
    popne {r0-r1}
    bxne lr

writeSaveToFile:
    push {r2,r3,r12,lr}
    bl sav_writeSaveToFile
    pop {r2,r3,r12,lr}
#endif
    pop {r0-r1}
    bx lr
