.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegLoad32Pointer index
    .if (\index << 2) == 0x00
        .short emu_ioFallbackLoad32
    .elseif (\index << 2) == 0x004
        .short emu_regDispStatVCountLoad32 // REG_DISPSTAT and REG_VCOUNT
    .elseif (\index << 2) == 0x100
        .short emu_timerCntLoad32 // REG_TM0CNT
    .elseif (\index << 2) == 0x104
        .short emu_timerCntLoad32 // REG_TM1CNT
    .elseif (\index << 2) == 0x108
        .short emu_timerCntLoad32 // REG_TM2CNT
    .elseif (\index << 2) == 0x10C
        .short emu_timerCntLoad32 // REG_TM3CNT
    .elseif (\index << 2) == 0x200
        .short emu_regIeIfLoad32 // REG_IE and REG_IF
    .else
        .short emu_ioFallbackLoad32
    .endif
.endm

.global memu_load32IoTable
memu_load32IoTable:
generate memu_ioRegLoad32Pointer 0x84
