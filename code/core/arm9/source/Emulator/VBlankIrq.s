.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_vblankIrq
    // For center and mask display capture has to be enabled every frame
    // and the buffers need to be swapped
    ldr sp,= dtcmIrqStackEnd
    push {r0-r1,lr}
    mov r0, #0x04000000
    ldr r1, [r0, #0x64] // REG_DISPCAPCNT
    eor r1, r1, #(1 << 16) // toggle between VRAM C and D
    orr r1, r1, #(1 << 31) // enable capture again
    str r1, [r0, #0x64]
    tst r1, #(1 << 16)
    moveq r1, #0x80 // LCDC
    movne r1, #0x84 // BG or OBJ
    strb r1, [r0, #0x242] // REG_VRAMCNT_C
    eor r1, r1, #4
    strb r1, [r0, #0x243] // REG_VRAMCNT_D
    pop {r0-r1,pc}
