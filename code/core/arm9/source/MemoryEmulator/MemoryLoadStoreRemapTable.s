.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.global memu_loadStoreRemapTable
memu_loadStoreRemapTable:
    .byte 0
    .byte (1 << 2)
    .byte (2 << 2)
    .byte (3 << 2)
    .byte (4 << 2)
    .byte (5 << 2)
    .byte (6 << 2)
    .byte (7 << 2)
    .byte (8 << 2)
    .byte (9 << 2)
    .byte (10 << 2)
    .byte (11 << 2)
    .byte (12 << 2)
    .byte (13 << 2)
    .byte (14 << 2)
    .byte (15 << 2)
.rept (256 - 16)
    .byte (1 << 2)
.endr

.global memu_armTmpUsrReg
memu_armTmpUsrReg:
    .word 0

.end
