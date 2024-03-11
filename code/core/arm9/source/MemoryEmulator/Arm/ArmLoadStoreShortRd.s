.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

.macro memu_armStrhRd rd
    arm_func memu_armStrhR\rd
        .if \rd < 8
            ldr r10, [r10, #memu_store16WordTable]
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd
            and r9, r9, r13, lsr #16 // r9 &= 0xFFFF
        .elseif \rd < 15
            ldr r10, [r10, #memu_store16WordTable]
            stmnedb r13, {r\rd}^
            nop
            ldrneh r9, [r13, #-4]
        .else
            mov r9, #0
            ldr r9, [r9, #memu_inst_addr]
            ldr r10, [r10, #memu_store16WordTable]
            add r9, r9, #4 // pc + 12
            and r9, r9, r13, lsr #16 // r9 &= 0xFFFF
        .endif
        blx r10
        memu_armReturn
.endm

generate memu_armStrhRd, 16

.macro memu_armLdrhRd rd
    arm_func memu_armLdrhR\rd
        .if \rd == 15
            memu_armReturn
            .mexit
        .endif

        ldr r10, [r10, #memu_load16WordTable]
        // interlock
        blx r10

        .if \rd < 8
            mov r\rd, r9
        .elseif \rd < 15
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
        .else
            // ldrh pc is not allowed
        .endif
        memu_armReturn
.endm

generate memu_armLdrhRd, 16

.macro memu_armLdrshRd rd
    arm_func memu_armLdrshR\rd
        .if \rd == 15
            memu_armReturn
            .mexit
        .endif

        ldr r11, [r10, #memu_load16WordTable]
        tst r8, #1
            bne memu_armLdrsbR\rd
        blx r11

        mov r9, r9, lsl #16
        .if \rd < 8
            mov r\rd, r9, asr #16
        .elseif \rd < 15
            mov r9, r9, asr #16
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
        .else
            // ldrsh pc is not allowed
        .endif
        memu_armReturn
.endm

generate memu_armLdrshRd, 16

.macro memu_armLdrsbRd rd
    arm_func memu_armLdrsbR\rd
        .if \rd == 15
            memu_armReturn
            .mexit
        .endif

        ldr r10, [r10, #memu_load8WordTable]
        // interlock
        blx r10

        mov r9, r9, lsl #24
        .if \rd < 8
            mov r\rd, r9, asr #24
        .elseif \rd < 15
            mov r9, r9, asr #24
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
        .else
            // ldrsb pc is not allowed
        .endif
        memu_armReturn
.endm

generate memu_armLdrsbRd, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armStrhRdTable
memu_armStrhRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrhR15 + 0x8000)

.global memu_armLdrhRdTable
memu_armLdrhRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrhR15 + 0x8000)

.global memu_armLdrshRdTable
memu_armLdrshRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrshR15 + 0x8000)

.global memu_armLdrsbRdTable
memu_armLdrsbRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrsbR15 + 0x8000)

.end
