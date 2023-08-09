.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegLoad16Pointer index
    .if (\index << 1) == 0x00
        .short emu_ioFallbackLoad16
    .else
        .short emu_ioFallbackLoad16
    .endif
.endm

.global memu_load16IoTable
memu_load16IoTable:
generate memu_ioRegLoad16Pointer 0x106
