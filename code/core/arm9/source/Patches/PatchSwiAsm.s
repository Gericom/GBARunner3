.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func patch_swiHandler
    cmp r13, #0xA0
        blo sav_swiHandler
    sub r13, r13, #(0xA0 + 1)
    ldr pc, [pc, r13, lsl #2]

.global patch_swiTable
patch_swiTable:
    .space 32 * 4

.end