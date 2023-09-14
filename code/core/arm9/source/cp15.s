.text
.arm

#include "AsmMacros.inc"

arm_func ic_invalidateAll
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0
    bx lr

arm_func dc_drainWriteBuffer
    mov r0, #0
    mcr p15, 0, r0, c7, c10, 4
    bx lr

arm_func dc_flushRange
    mov r12, #0
    add r1, r1, r0
    bic r0, r0, #0x1F
1:
    mcr p15, 0, r12, c7, c10, 4 // drain write buffer
    mcr p15, 0, r0, c7, c14, 1 // flush line
    add r0, r0, #32
    cmp r0, r1
    blt 1b
    bx lr

arm_func dc_invalidateRange
    add r1, r1, r0
    bic r0, r0, #0x1F
1:
    mcr p15, 0, r0, c7, c6, 1
    add r0, r0, #32
    cmp r0, r1
    blt 1b
    bx lr
