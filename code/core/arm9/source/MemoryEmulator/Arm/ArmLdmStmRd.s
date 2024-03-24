.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

// r8 = number of registers to write
// r9 = 1 << Rn
// r10 = start offset
// r11 = &memu_armStmRd
// r12 = value of Rn
// sp = stack pointer
// lr = instruction
arm_func memu_armStmRd
    rsb r11, lr, #0
    and r11, r11, lr
    cmp r11, r9

    mov r9, r12

    rsb r12, r8, #15
    add r8, r9, r10
    sub r11, r8, r12, lsl #2
    mov r12, r12, lsl #4
    addeq r12, r12, #4 // skip first load of r9

    mov r10, r8, lsr #24
    add r11, r11, #(14 * 4)
    cmp r10, r11, lsr #24
    ldrb r10, [r10, #memu_loadStoreRemapTable]
    strh lr, 1f
        ldrne r11,= memu_store32
        ldreq r11, [r10, #memu_store32WordTable]

    movs lr, lr, lsl #16
1:
    stmnedb sp, {r0}^ // to be modified
    nop
    str r11, [sp, #-(16 << 2)]!
    add pc, pc, r12
    nop

.macro memu_armStmRd_store index
    ldr r9, [sp, #(4 + (\index << 2))]
    blx r11
.if \index < 14
    ldr r11, [sp]
    add r8, r8, #4
.endif
.endm

generate memu_armStmRd_store, 15

    add sp, sp, #(16 << 2)
    memu_armReturn

// r8 = number of registers to write
// r9 = 1 << Rn
// r10 = start offset
// r11 = &memu_armStmRdWithPc
// r12 = value of Rn
// sp = stack pointer
// lr = instruction
arm_func memu_armStmRdWithPc
    bic lr, lr, #(1 << 15)

    rsb r11, lr, #0
    and r11, r11, lr
    cmp r11, r9

    mov r9, r12

    rsb r12, r8, #16
    add r8, r9, r10
    sub r11, r8, r12, lsl #2
    mov r12, r12, lsl #4
    addeq r12, r12, #4 // skip first load of r9

    mov r10, r8, lsr #24
    add r11, r11, #(15 * 4)
    cmp r10, r11, lsr #24
    ldrb r10, [r10, #memu_loadStoreRemapTable]
    strh lr, 1f
        ldrne r11,= memu_store32
        ldreq r11, [r10, #memu_store32WordTable]

    movs lr, lr, lsl #16
1:
    stmnedb sp, {r0}^ // to be modified
    nop
    str r11, [sp, #-(16 << 2)]!
    add pc, pc, r12
    nop

.macro memu_armStmRdWithPc_store index
    ldr r9, [sp, #(4 + (\index << 2))]
    blx r11
    ldr r11, [sp]
    add r8, r8, #4
.endm

generate memu_armStmRdWithPc_store, 15

    mov r9, #0
    ldr r9, [r9, #memu_inst_addr]
    add r9, r9, #4
    blx r11

    add sp, sp, #(16 << 2)
    memu_armReturn

// r8 = number of registers to read
// r9 = 1 << Rn
// r10 = start offset
// r11 = &memu_armLdmRd
// r12 = value of Rn
// sp = stack pointer
// lr = instruction
arm_func memu_armLdmRd
    add r12, r12, r10
    mov r9, r12, lsr #24
    ldrb r10, [r9, #memu_loadStoreRemapTable]
    add r11, r12, r8, lsl #2
    sub r11, r11, #4
    cmp r9, r11, lsr #24
        ldrne r11,= memu_load32
        ldreq r11, [r10, #memu_load32WordTable]

    rsb r9, r8, #15
    bic r8, r12, #3
    strh lr, 1f
    str r11, [sp, #-(16 << 2)]!

    add pc, pc, r9, lsl #4
    nop

.macro memu_armLdmRd_load index
    blx r11
    str r9, [sp, #(4 + (\index << 2))]
.if \index < 14
    ldr r11, [sp]
    add r8, r8, #4
.endif
.endm

generate memu_armLdmRd_load, 15
    add sp, sp, #(16 << 2)
1:
    ldmdb sp, {r0}^ // modified
    memu_armReturn

// r8 = value of Rn
// r9 = 1 << Rn
// r10 = start offset
// r11 = &memu_armLdmRdWithPc
// r12 = number of registers to read
// sp = stack pointer
// lr = instruction
arm_func memu_armLdmRdWithPc
    add r12, r12, r10
    mov r9, r12, lsr #24
    ldrb r10, [r9, #memu_loadStoreRemapTable]
    add r11, r12, r8, lsl #2
    sub r11, r11, #4
    cmp r9, r11, lsr #24
        ldrne r11,= memu_load32
        ldreq r11, [r10, #memu_load32WordTable]

    rsb r9, r8, #16
    bic r8, r12, #3
    mov r9, r9, lsl #4
    movs r12, lr, lsl #17
        addeq r9, r9, #8 // no other register than pc
        bicne lr, lr, #(1 << 15)
        strneh lr, 1f
    str r11, [sp, #-(16 << 2)]!

    add pc, pc, r9
    nop

.macro memu_armLdmRdWithPc_load index
    blx r11
    str r9, [sp, #(4 + (\index << 2))]
    ldr r11, [sp]
    add r8, r8, #4
.endm

generate memu_armLdmRdWithPc_load, 14

    blx r11
    str r9, [sp, #(4 + (14 << 2))]
    ldr r11, [sp], #(16 << 2)
1:
    ldmdb sp, {r0}^ // modified
    nop
    add r8, r8, #4
    blx r11
    str r9, addressToJumpTo
    msr cpsr_c, #0xD7
    ldr lr, addressToJumpTo
    movs pc, lr

addressToJumpTo:
    .word 0

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
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc
    .word memu_armStmRdWithPc

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
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
    .word memu_armLdmRdWithPc
