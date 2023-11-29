.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdmiaRn rn
    .balign 8
    arm_func memu_thumbLdmiaR\rn
        ldr r12,= gPopCountTable
        and r13, r11, #0xFF
        ldrb r12, [r12, r13]

        bic r8, r\rn, #3

        add r\rn, r\rn, r12, lsl #2

        b memu_thumbLdmiaCommon
.endm

generate memu_thumbLdmiaRn, 8

arm_func memu_thumbLdmiaCommon
    .macro memu_thumbLdmiaCommon_loadReg reg
        tst r13, #(1 << \reg)
        beq 1f
        bl memu_load32
        mov r\reg, r9
        add r8, r8, #4
        1:
    .endm

    generate memu_thumbLdmiaCommon_loadReg, 8

    memu_thumbReturn

.macro memu_thumbStmiaRn rn
    .balign 8
    arm_func memu_thumbStmiaR\rn
        ldr r12,= gPopCountTable
        and r13, r11, #0xFF
        ldrb r12, [r12, r13]

        rsb r11, r13, #0
        and r11, r11, r13
        cmp r11, #(1 << \rn)

        and r11, r\rn, #3
        orr r13, r11, lsl #16

        sub r8, r\rn, #4

        addne r\rn, r\rn, r12, lsl #2


        .macro memu_thumbStmiaR\rn\()_storeReg reg
            tst r13, #(1 << \reg)
            movne r9, r\reg
            addne r8, r8, #4
            blne memu_store32
        .endm

        generate memu_thumbStmiaR\rn\()_storeReg, 8

        add r8, r8, #4
        bic r8, r8, #3
        orr r\rn, r8, r13, lsr #16
        memu_thumbReturn
.endm

generate memu_thumbStmiaRn, 8

.end
