.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_instructionPointer2 type, i, p, u, b, w, l
    .if (\type == 0b01)
        // ldr, ldrb, str, strb
        .if \i == 0
            .word memu_armLoadStoreImmTable_\u
        .else
            .word memu_armLoadStoreRmTable_\u
        .endif
        .word memu_armLoadStoreRnTable_\p\w
        .if (\b == 0) && (\l == 0)
            .word memu_armStrRdTable
        .elseif (\b == 0) && (\l == 1)
            .word memu_armLdrRdTable
        .elseif (\b == 1) && (\l == 0)
            .word memu_armStrbRdTable
        .else
            .word memu_armLdrbRdTable
        .endif
    .elseif (\type == 0b10) && (\i == 0)
        // ldm/stm
        @ .word memu_armLdmStmRmTable
        @ .word memu_armLdmStmRnTable_\p\u\w
        @ .word memu_armLdmStmRdTable
        .word 0
        .word 0
        .word 0
    .elseif (\type == 0b10) && (\i == 1)
        // ldrh/strh
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .word memu_armLoadStoreRnTable_\p\w
        .if \l == 0
            .word memu_armStrhRdTable
        .else
            .word memu_armLdrhRdTable
        .endif
    .elseif (\type == 0b11) && (\i == 0) && (\l == 1)
        // ldrsb
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .word memu_armLoadStoreRnTable_\p\w
        .word memu_armLdrsbRdTable
    .elseif (\type == 0b11) && (\i == 1) && (\l == 1)
        // ldrsh
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .word memu_armLoadStoreRnTable_\p\w
        .word memu_armLdrshRdTable
    .else
        .word 0
        .word 0
        .word 0
    .endif
    .word 0
.endm

.macro memu_instructionPointer index
    //         76543210
    // index = ttipubwl
    memu_instructionPointer2 %((\index>>6)&3), %((\index>>5)&1), %((\index>>4)&1), %((\index>>3)&1), %((\index>>2)&1), %((\index>>1)&1), %((\index>>0)&1)
.endm

.global memu_armTmpUsrReg
memu_armTmpUsrReg:
    .word 0

.global memu_armDispatchTable
memu_armDispatchTable:

generate memu_instructionPointer, 256

.end
