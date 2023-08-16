.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ArmMacros.inc"
#include "../MemoryEmuDtcm.inc"

.macro memu_armStrhRd rd
    arm_func memu_armStrhR\rd
        .if \rd < 8
            movne r9, r\rd // if Rd is not equal to Rn, get the value of Rd
            mov r9, r9, lsl #16
            mov r9, r9, lsr #16

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x84] // memu_store16Table

            bic r8, r8, #1
        .elseif \rd < 15
            stmnedb r13, {r\rd}^
            nop
            ldrneh r9, [r13, #-4]

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x84] // memu_store16Table

            bic r8, r8, #1
        .else
            mov r9, #memu_inst_addr
            ldr r9, [r9]
            bic r8, r8, #1
            add r9, r9, #4 // pc + 12
            mov r9, r9, lsl #16

            and r10, r8, #0x0F000000
            cmp r8, #0x10000000
            addlo r10, r13, r10, lsr #22
            ldrlo r10, [r10, #-0x84] // memu_store16Table

            mov r9, r9, lsr #16
        .endif
        blxlo r10
        memu_armReturn
.endm

generate memu_armStrhRd, 16

.macro memu_armLdrhRd rd
    arm_func memu_armLdrhR\rd
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r13, r10, lsr #22
        ldrlo r10, [r9, #-0x144] // memu_load16Table
        ldrhs r10,= memu_load16Undefined
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
        tst r8, #1
            bne memu_armLdrsbR\rd
        
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r13, r10, lsr #22
        ldrlo r10, [r9, #-0x144] // memu_load16Table
        ldrhs r10,= memu_load16Undefined
        blx r10

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
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
        addlo r9, r13, r10, lsr #22
        ldrlo r10, [r9, #-0x104] // memu_load8Table
        ldrhs r10,= memu_load8Undefined
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

.global memu_armStrhRdTable
memu_armStrhRdTable:
    .word memu_armStrhR0
    .word memu_armStrhR1
    .word memu_armStrhR2
    .word memu_armStrhR3
    .word memu_armStrhR4
    .word memu_armStrhR5
    .word memu_armStrhR6
    .word memu_armStrhR7
    .word memu_armStrhR8
    .word memu_armStrhR9
    .word memu_armStrhR10
    .word memu_armStrhR11
    .word memu_armStrhR12
    .word memu_armStrhR13
    .word memu_armStrhR14
    .word memu_armStrhR15

.global memu_armLdrhRdTable
memu_armLdrhRdTable:
    .word memu_armLdrhR0
    .word memu_armLdrhR1
    .word memu_armLdrhR2
    .word memu_armLdrhR3
    .word memu_armLdrhR4
    .word memu_armLdrhR5
    .word memu_armLdrhR6
    .word memu_armLdrhR7
    .word memu_armLdrhR8
    .word memu_armLdrhR9
    .word memu_armLdrhR10
    .word memu_armLdrhR11
    .word memu_armLdrhR12
    .word memu_armLdrhR13
    .word memu_armLdrhR14
    .word memu_armLdrhR15

.global memu_armLdrshRdTable
memu_armLdrshRdTable:
    .word memu_armLdrshR0
    .word memu_armLdrshR1
    .word memu_armLdrshR2
    .word memu_armLdrshR3
    .word memu_armLdrshR4
    .word memu_armLdrshR5
    .word memu_armLdrshR6
    .word memu_armLdrshR7
    .word memu_armLdrshR8
    .word memu_armLdrshR9
    .word memu_armLdrshR10
    .word memu_armLdrshR11
    .word memu_armLdrshR12
    .word memu_armLdrshR13
    .word memu_armLdrshR14
    .word memu_armLdrshR15

.global memu_armLdrsbRdTable
memu_armLdrsbRdTable:
    .word memu_armLdrsbR0
    .word memu_armLdrsbR1
    .word memu_armLdrsbR2
    .word memu_armLdrsbR3
    .word memu_armLdrsbR4
    .word memu_armLdrsbR5
    .word memu_armLdrsbR6
    .word memu_armLdrsbR7
    .word memu_armLdrsbR8
    .word memu_armLdrsbR9
    .word memu_armLdrsbR10
    .word memu_armLdrsbR11
    .word memu_armLdrsbR12
    .word memu_armLdrsbR13
    .word memu_armLdrsbR14
    .word memu_armLdrsbR15

.end
