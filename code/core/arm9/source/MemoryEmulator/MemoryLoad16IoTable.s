.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegLoad16Pointer index
    .if (\index << 1) == 0x000
        .short emu_ioFallbackLoad16
    .elseif (\index << 1) == 0x006
        .short emu_regVCountLoad16 // REG_VCOUNT
    .elseif (\index << 1) == 0x200
        .short emu_ioFallbackLoad16 // REG_IE
    .elseif (\index << 1) == 0x202
        .short emu_regIfLoad16 // REG_IF
    .elseif (\index << 1) == 0x204
        .short memu_load16Undefined // REG_WAITCNT
    .elseif (\index << 1) == 0x208
        .short emu_ioFallbackLoad16 // REG_IME
    .else
        .short emu_ioFallbackLoad16
    .endif
.endm

.global memu_load16IoTable
memu_load16IoTable:
generate memu_ioRegLoad16Pointer 0x106
