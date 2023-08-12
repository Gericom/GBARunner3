.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_regVCountLoad16
    mov r9, #0x04000000
    ldrh r9, [r9, #6]
	cmp r9, #160
	    blt 1f
	cmp r9, #192
	    movlt r9, #159
	    blt 1f
	sub r9, r9, #32
	cmp r9, #227
	    movgt r9, #227
1:
    tst r8, #1
        movne r9, r9, ror #8
	bx lr
