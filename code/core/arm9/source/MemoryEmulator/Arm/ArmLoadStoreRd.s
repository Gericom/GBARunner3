.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "../MemoryEmuDtcm.inc"

.macro memu_armStrRd rd
    arm_func memu_armStrR\rd
        .if \rd < 8
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd
            
            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0xC4] // memu_store32Table
            
            bic r8, r8, #3
        .elseif \rd < 15
            stmnedb r13, {r\rd}^
            nop
            ldrne r9, [r13, #-4]

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0xC4] // memu_store32Table

            bic r8, r8, #3
        .else
            mov r9, #memu_inst_addr
            ldr r9, [r9]
            bic r8, r8, #3

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0xC4] // memu_store32Table

            add r9, r9, #4 // pc + 12
        .endif

        blxlo r10

        memu_armReturn
.endm

generate memu_armStrRd, 16

.macro memu_armStrbRd rd
    arm_func memu_armStrbR\rd
        .if \rd < 8
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x44] // memu_store8Table

            and r9, r9, #0xFF
        .elseif \rd < 15
            stmnedb r13, {r\rd}^
            nop
            ldrneb r9, [r13, #-4]

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x44] // memu_store8Table
            // interlock
        .else
            mov r9, #memu_inst_addr
            ldr r9, [r9]
            add r9, r9, #4 // pc + 12

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x44] // memu_store8Table

            and r9, r9, #0xFF
        .endif
        blxlo r10
        memu_armReturn
.endm

generate memu_armStrbRd, 16

.macro memu_armLdrRd rd
    arm_func memu_armLdrR\rd
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r10, r13, r10, lsr #22
        ldrlo r10, [r10, #-0x184] // memu_load32Table
        ldrhs r10,= memu_load32Undefined
        blx r10

        .if \rd < 8
            mov r\rd, r9
            memu_armReturn
        .elseif \rd < 15
            str r9, [r13, #-4]
            ldmdb r13, {r\rd}^
            memu_armReturn
        .else
            mov r8, #memu_inst_addr
            str r9, [r8]
            msr cpsr_c, #0xD7
            mov lr, #memu_inst_addr
            ldr lr, [lr]
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

        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r10, r13, r10, lsr #22
        ldrlo r10, [r10, #-0x104] // memu_load8Table
        ldrhs r10,= memu_load8Undefined
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

.global memu_armStrRdTable
memu_armStrRdTable:
    .word memu_armStrR0
    .word memu_armStrR1
    .word memu_armStrR2
    .word memu_armStrR3
    .word memu_armStrR4
    .word memu_armStrR5
    .word memu_armStrR6
    .word memu_armStrR7
    .word memu_armStrR8
    .word memu_armStrR9
    .word memu_armStrR10
    .word memu_armStrR11
    .word memu_armStrR12
    .word memu_armStrR13
    .word memu_armStrR14
    .word memu_armStrR15

.global memu_armStrbRdTable
memu_armStrbRdTable:
    .word memu_armStrbR0
    .word memu_armStrbR1
    .word memu_armStrbR2
    .word memu_armStrbR3
    .word memu_armStrbR4
    .word memu_armStrbR5
    .word memu_armStrbR6
    .word memu_armStrbR7
    .word memu_armStrbR8
    .word memu_armStrbR9
    .word memu_armStrbR10
    .word memu_armStrbR11
    .word memu_armStrbR12
    .word memu_armStrbR13
    .word memu_armStrbR14
    .word memu_armStrbR15

.global memu_armLdrRdTable
memu_armLdrRdTable:
    .word memu_armLdrR0
    .word memu_armLdrR1
    .word memu_armLdrR2
    .word memu_armLdrR3
    .word memu_armLdrR4
    .word memu_armLdrR5
    .word memu_armLdrR6
    .word memu_armLdrR7
    .word memu_armLdrR8
    .word memu_armLdrR9
    .word memu_armLdrR10
    .word memu_armLdrR11
    .word memu_armLdrR12
    .word memu_armLdrR13
    .word memu_armLdrR14
    .word memu_armLdrR15

.global memu_armLdrbRdTable
memu_armLdrbRdTable:
    .word memu_armLdrbR0
    .word memu_armLdrbR1
    .word memu_armLdrbR2
    .word memu_armLdrbR3
    .word memu_armLdrbR4
    .word memu_armLdrbR5
    .word memu_armLdrbR6
    .word memu_armLdrbR7
    .word memu_armLdrbR8
    .word memu_armLdrbR9
    .word memu_armLdrbR10
    .word memu_armLdrbR11
    .word memu_armLdrbR12
    .word memu_armLdrbR13
    .word memu_armLdrbR14
    .word memu_armLdrbR15
