.text
.arm

#include "AsmMacros.inc"

arm_func mpu_setRegionICacheEnable
    mov r2, #1
    mov r0, r2, lsl r0
    cmp r1, #0
    mrc p15, 0, r2, c2, c0, 1
    biceq r2, r2, r0
    orrne r2, r2, r0
    mcr p15, 0, r2, c2, c0, 1
    bx lr
