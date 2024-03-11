.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/MemoryLoadStoreTableDefs.inc"

.macro memu_armStrRd rd
    arm_func memu_armStrR\rd
        .if \rd < 8
            ldr r10, [r10, #memu_store32WordTable]
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd
        .elseif \rd < 15
            ldr r10, [r10, #memu_store32WordTable]

            stmnedb r13, {r\rd}^
            nop
            ldrne r9, [r13, #-4]
        .else
            mov r9, #0
            ldr r9, [r9, #memu_inst_addr]

            ldr r10, [r10, #memu_store32WordTable]
            add r9, r9, #4 // pc + 12
        .endif

        blx r10

        memu_armReturn
.endm

generate memu_armStrRd, 16

.macro memu_armStrbRd rd
    arm_func memu_armStrbR\rd
        .if \rd < 8
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd
            ldr r10, [r10, #memu_store8WordTable]
            and r9, r9, #0xFF
        .elseif \rd < 15
            ldr r10, [r10, #memu_store8WordTable]

            stmnedb r13, {r\rd}^
            nop
            ldrneb r9, [r13, #-4]
        .else
            mov r9, #0
            ldr r9, [r9, #memu_inst_addr]
            ldr r10, [r10, #memu_store8WordTable]
            add r9, r9, #4 // pc + 12
            and r9, r9, #0xFF
        .endif

        blx r10

        memu_armReturn
.endm

generate memu_armStrbRd, 16

.macro memu_armLdrRd rd
    arm_func memu_armLdrR\rd
        ldr r10, [r10, #memu_load32WordTable]
        // interlock
        blx r10

        .if \rd < 8
            mov r\rd, r9
            memu_armReturn
        .elseif \rd < 15
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
            memu_armReturn
        .else
            mov r8, #0
            str r9, [r8, #memu_inst_addr]
            msr cpsr_c, #0xD7
            mov lr, #0
            ldr lr, [lr, #memu_inst_addr]
            movs pc, lr
        .endif
.endm

generate memu_armLdrRd, 16

.macro memu_armLdrbRd rd
    arm_func memu_armLdrbR\rd
        .if \rd == 15
            memu_armReturn
            .mexit
        .endif

        ldr r10, [r10, #memu_load8WordTable]
        // interlock
        blx r10

        .if \rd < 8
            and r\rd, r9, #0xFF
        .elseif \rd < 15
            and r9, r9, #0xFF
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
        .else
            // ldrb pc is not allowed
        .endif
        memu_armReturn
.endm

generate memu_armLdrbRd, 16

.section ".dtcm", "aw"

.balign 64

.global memu_armStrRdTable
memu_armStrRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armStrR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrR15 + 0x8000)

.global memu_armStrbRdTable
memu_armStrbRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armStrbR15 + 0x8000)

.global memu_armLdrRdTable
memu_armLdrRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrR15 + 0x8000)

.global memu_armLdrbRdTable
memu_armLdrbRdTable:
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR0 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR1 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR2 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR3 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR4 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR5 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR6 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR7 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR8 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR9 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR10 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR11 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR12 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR13 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR14 + 0x8000)
    .short memu_loadStoreRemapTable
    .short (memu_armLdrbR15 + 0x8000)
