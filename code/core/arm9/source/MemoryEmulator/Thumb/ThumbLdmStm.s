.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

.macro memu_thumbLdmiaRn rn
    .balign 8
    arm_func memu_thumbLdmiaR\rn
        ldr r12,= gPopCountTable
        and r11, r11, #0xFF
        ldrb r12, [r12, r11]

        bic r8, r\rn, #3
        mov r9, r8, lsr #24

        add r\rn, r\rn, r12, lsl #2

        b memu_thumbLdmiaCommon
.endm

generate memu_thumbLdmiaRn, 8

// r8 = start address
// r12 = number of registers to read
arm_func memu_thumbLdmiaCommon
    add r10, r8, r12, lsl #2
    sub r10, r10, #4
    cmp r9, r10, lsr #24
        ldreqb r10, [r9, #memu_loadStoreRemapTable]
        bne memu_thumbLdmiaCommonRegionCrossing

    strb r11, 1f
    ldr r11, [r10, #memu_load32WordTable]
    rsb r9, r12, #8
    str r11, [sp, #-(9 << 2)]!

    add pc, pc, r9, lsl #4
    nop

.macro memu_thumbLdmiaCommon_loadReg index
    blx r11
    str r9, [sp, #(4 + (\index << 2))]
.if \index < 7
    ldr r11, [sp]
    add r8, r8, #4
.endif
.endm

    generate memu_thumbLdmiaCommon_loadReg, 8

    add sp, sp, #(9 << 2)
1:
    ldmdb sp, {r0} // modified
    memu_thumbReturn

// r8 = start address
// r12 = number of registers to read
arm_func memu_thumbLdmiaCommonRegionCrossing
    strb r11, 1f
    rsb r9, r12, #8
    sub sp, sp, #(8 << 2)

    add r9, r9, r9, lsl #1
    add pc, pc, r9, lsl #2
    nop

.macro memu_thumbLdmiaCommonRegionCrossing_loadReg index
    bl memu_load32
    str r9, [sp, #(\index << 2)]
.if \index < 7
    add r8, r8, #4
.endif
.endm

    generate memu_thumbLdmiaCommonRegionCrossing_loadReg, 8

    add sp, sp, #(8 << 2)
1:
    ldmdb sp, {r0} // modified
    memu_thumbReturn

.macro memu_thumbStmiaRn rn
    .balign 8
    arm_func memu_thumbStmiaR\rn
        strb r11, 1f
        ldr r12,= gPopCountTable
        and r11, r11, #0xFF
        ldrb r12, [r12, r11]

        rsb r8, r11, #0
        and r8, r8, r11
        cmp r8, #(1 << \rn)

        mov r8, r\rn
        addne r\rn, r\rn, r12, lsl #2
    1:
        stmdb sp, {r0} // to be modified
        addeq r\rn, r\rn, r12, lsl #2

        b memu_thumbStmiaCommon
.endm

generate memu_thumbStmiaRn, 8

arm_func memu_thumbStmiaCommon
    mov r9, r8, lsr #24
    ldrb r10, [r9, #memu_loadStoreRemapTable]
    add r11, r8, r12, lsl #2
    sub r11, r11, #4
    cmp r9, r11, lsr #24
        bne memu_thumbStmiaCommonRegionCrossing

    ldr r11, [r10, #memu_store32WordTable]
    rsb r12, r12, #8
    str r11, [sp, #-(9 << 2)]!
    add pc, pc, r12, lsl #4
    nop

.macro memu_thumbStmiaCommon_storeReg index
    ldr r9, [sp, #(4 + (\index << 2))]
    blx r11
.if \index < 7
    ldr r11, [sp]
    add r8, r8, #4
.endif
.endm

    generate memu_thumbStmiaCommon_storeReg, 8

    add sp, sp, #(9 << 2)
    memu_thumbReturn

arm_func memu_thumbStmiaCommonRegionCrossing
    sub sp, sp, #(8 << 2)
    rsb r12, r12, #8
    add r12, r12, r12, lsl #1
    add pc, pc, r12, lsl #2
    nop

.macro memu_thumbStmiaCommonRegionCrossing_storeReg index
    ldr r9, [sp, #(\index << 2)]
    bl memu_store32
.if \index < 7
    add r8, r8, #4
.endif
.endm

    generate memu_thumbStmiaCommonRegionCrossing_storeReg, 8

    add sp, sp, #(8 << 2)
    memu_thumbReturn

.end
