.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
    
.macro memu_armLdmStmRm_pu p, u
arm_func memu_armLdmStmRm_\p\u
    //count nr bits
    ldr r9,= gPopCountTable
    and lr, lr, sp, lsr #16
    and r8, lr, #0xFF
    ldrb r8, [r9, r8]
    ldrb r9, [r9, lr, lsr #8]
    and r10, r10, lr, lsr #10 // r10 = Rd << 2
    ldr r11, [r10, -r11, lsl #6] // r11 = Rd handler address
    add r8, r9, r8

    mov r10, #0
    .if \u == 0
        sub r10, r10, r8, lsl #2
        mov r9, r10 // for writeback
    .else
        mov r9, r8, lsl #2
    .endif

    .if \p == \u
        add r10, r10, #4 // IB and DA
    .endif

    bx r12
.endm

memu_armLdmStmRm_pu 0, 0
memu_armLdmStmRm_pu 0, 1
memu_armLdmStmRm_pu 1, 0
memu_armLdmStmRm_pu 1, 1

.section ".dtcm", "aw"

.balign 64

.global memu_armLdmStmRmTable_00
memu_armLdmStmRmTable_00:
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00
    .word memu_armLdmStmRm_00

.global memu_armLdmStmRmTable_01
memu_armLdmStmRmTable_01:
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01
    .word memu_armLdmStmRm_01

.global memu_armLdmStmRmTable_10
memu_armLdmStmRmTable_10:
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10
    .word memu_armLdmStmRm_10

.global memu_armLdmStmRmTable_11
memu_armLdmStmRmTable_11:
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
    .word memu_armLdmStmRm_11
