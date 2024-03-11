.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func memu_armStmRd
    mov r9, r8
    add r8, r8, r10

    rsb r10, lr, #0
    and r10, r10, lr
    cmp r10, r12

    biceq lr, lr, r10
    bic r13, r13, lr, lsl #15
    orr r13, r13, #0x80000000

    subne r8, r8, #4
    bleq memu_store32

    .macro memu_armStmRd_storeLoReg reg
        tst r13, #(1 << (\reg + 15))
        moveq r9, r\reg
        addeq r8, r8, #4
        bleq memu_store32
    .endm

    generate memu_armStmRd_storeLoReg, 8

    .macro memu_armStmRd_storeHiReg reg
        tst r13, #(1 << (\reg + 15))
        stmeqdb r13, {r\reg}^
        bne 1f
        ldr r9, [r13, #-4]
        add r8, r8, #4
        bl memu_store32
    1:
    .endm

    generate memu_armStmRd_storeHiReg 15, 8

    tst r13, #(1 << 30)
    bne 1f
    mov r9, #0
    ldr r9, [r9, #memu_inst_addr]
    add r9, r9, #4
    add r8, r8, #4
    bl memu_store32
1:
    memu_armReturn

arm_func memu_armLdmRd
    bic r13, r13, lr, lsl #15
    orr r13, r13, #0x80000000
    add r8, r8, r10
    bic r8, r8, #3

    .macro memu_armLdmRd_storeLoReg reg
        tst r13, #(1 << (\reg + 15))
        bne 1f
        bl memu_load32
        mov r\reg, r9
        add r8, r8, #4
    1:
    .endm

    generate memu_armLdmRd_storeLoReg, 8

    .macro memu_armLdmRd_storeHiReg reg
        tst r13, #(1 << (\reg + 15))
        bne 1f
        bl memu_load32
        str r9, [r13, #-4]
        ldmdb r13, {r\reg}^
        nop
        add r8, r8, #4
    1:
    .endm

    generate memu_armLdmRd_storeHiReg 15, 8

    tst r13, #(1 << 30)
    bne 1f
    bl memu_load32
    str r9, [r13, #-4]
    msr cpsr_c, #0xD7
    ldr lr,= memu_armTmpUsrReg
    ldr lr, [lr]
    movs pc, lr
1:
    memu_armReturn

.section ".dtcm", "aw"

.balign 64

.global memu_armStmRdTable
memu_armStmRdTable:
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd
    .word memu_armStmRd

.global memu_armLdmRdTable
memu_armLdmRdTable:
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
    .word memu_armLdmRd
