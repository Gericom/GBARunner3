.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro memu_armLdmStmRn rn, w
    arm_func memu_armLdmStmR\rn\()_\w
        .if \rn < 8
            mov r12, r\rn
        .elseif \rn < 15
            stmdb sp, {r\rn}^
            ldr r12, [sp, #-4]
        .else
            
        .endif

        .if \w == 1
            .if \rn < 8
                add r\rn, r9, r12
            .elseif \rn < 15
                add r9, r9, r12
                str r9, [sp, #-4]
                ldmdb sp, {r\rn}^
            .else

            .endif
        .endif

        mov r9, #(1 << \rn)
        bx r11
.endm

.macro memu_armLdmStmRn_w rn
    memu_armLdmStmRn rn, 0
    memu_armLdmStmRn rn, 1
.endm

generate memu_armLdmStmRn_w, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armLdmStmRnTable_0
memu_armLdmStmRnTable_0:
    .word memu_armLdmStmR0_0
    .word memu_armLdmStmR1_0
    .word memu_armLdmStmR2_0
    .word memu_armLdmStmR3_0
    .word memu_armLdmStmR4_0
    .word memu_armLdmStmR5_0
    .word memu_armLdmStmR6_0
    .word memu_armLdmStmR7_0
    .word memu_armLdmStmR8_0
    .word memu_armLdmStmR9_0
    .word memu_armLdmStmR10_0
    .word memu_armLdmStmR11_0
    .word memu_armLdmStmR12_0
    .word memu_armLdmStmR13_0
    .word memu_armLdmStmR14_0
    .word memu_armLdmStmR15_0

.global memu_armLdmStmRnTable_1
memu_armLdmStmRnTable_1:
    .word memu_armLdmStmR0_1
    .word memu_armLdmStmR1_1
    .word memu_armLdmStmR2_1
    .word memu_armLdmStmR3_1
    .word memu_armLdmStmR4_1
    .word memu_armLdmStmR5_1
    .word memu_armLdmStmR6_1
    .word memu_armLdmStmR7_1
    .word memu_armLdmStmR8_1
    .word memu_armLdmStmR9_1
    .word memu_armLdmStmR10_1
    .word memu_armLdmStmR11_1
    .word memu_armLdmStmR12_1
    .word memu_armLdmStmR13_1
    .word memu_armLdmStmR14_1
    .word memu_armLdmStmR15_1
    