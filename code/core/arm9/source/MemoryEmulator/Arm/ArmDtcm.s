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
        .short memu_armLoadStoreRnTable_\p\w
        .if (\b == 0) && (\l == 0)
            .short memu_armStrRdTable
        .elseif (\b == 0) && (\l == 1)
            .short memu_armLdrRdTable
        .elseif (\b == 1) && (\l == 0)
            .short memu_armStrbRdTable
        .else
            .short memu_armLdrbRdTable
        .endif
    .elseif (\type == 0b00) && (\i == 0)
        // ldm/stm
        .word memu_armLdmStmRmTable_\p\u
        .short memu_armLdmStmRnTable_\w
        .if \l == 1
            .short memu_armLdmRdTable
        .else
            .short memu_armStmRdTable
        .endif
    .elseif (\type == 0b10) && (\i == 0)
        // swp/swpb
        .word memu_armLoadStoreShortRmTable_1
        .short memu_armSwpRnTable
        .if (\b == 0)
            .short memu_armSwpRdTable
        .else
            .short memu_armSwpbRdTable
        .endif
    .elseif (\type == 0b10) && (\i == 1)
        // ldrh/strh
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .short memu_armLoadStoreRnTable_\p\w
        .if \l == 0
            .short memu_armStrhRdTable
        .else
            .short memu_armLdrhRdTable
        .endif
    .elseif (\type == 0b11) && (\i == 0) && (\l == 1)
        // ldrsb
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .short memu_armLoadStoreRnTable_\p\w
        .short memu_armLdrsbRdTable
    .elseif (\type == 0b11) && (\i == 1) && (\l == 1)
        // ldrsh
        .if \b == 0
            .word memu_armLoadStoreShortRmTable_\u
        .else
            .word memu_armLoadStoreShortImmTable_\u
        .endif
        .short memu_armLoadStoreRnTable_\p\w
        .short memu_armLdrshRdTable
    .else
        .word 0
        .short 0
        .short 0
    .endif
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
