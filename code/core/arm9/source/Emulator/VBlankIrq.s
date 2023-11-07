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
    pop {r0-r1}
    bx lr
