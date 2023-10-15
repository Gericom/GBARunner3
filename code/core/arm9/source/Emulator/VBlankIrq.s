.text
.altmacro

#include "AsmMacros.inc"

arm_func emu_vblankIrq
    mov r13, #0x04000000
    ldr r13, [r13, #0x130]
    tst r13, #(1 << 2) // select button
    bxne lr
    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r12,lr}
#ifndef GBAR3_TEST
    bl jit_disable
#endif
    pop {r0-r3,r12,pc}
