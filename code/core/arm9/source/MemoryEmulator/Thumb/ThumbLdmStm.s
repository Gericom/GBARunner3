.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdmiaRn rn
    arm_func memu_thumbLdmiaR\rn
        and r13, lr, #0xFF
        ldr r12,= gPopCountTable
        ldrb r12, [r12, r13]

        bic r8, r\rn, #3

        add r\rn, r8, r12, lsl #2

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
    arm_func memu_thumbStmiaR\rn
        and r13, lr, #0xFF
        ldr r12,= gPopCountTable
        ldrb r12, [r12, r13]

        bic r8, r\rn, #3

        rsb r11, r13, #0
        and r11, r11, r13
        cmp r11, #(1 << \rn)
        addne r\rn, r8, r12, lsl #2

        sub r8, r8, #4

        .macro memu_thumbStmiaR\rn\()_storeReg reg
            tst r13, #(1 << \reg)
            movne r9, r\reg
            addne r8, r8, #4
            blne memu_store32
        .endm

        generate memu_thumbStmiaR\rn\()_storeReg, 8

        add r\rn, r8, #4
        memu_thumbReturn
.endm

generate memu_thumbStmiaRn, 8

.end
