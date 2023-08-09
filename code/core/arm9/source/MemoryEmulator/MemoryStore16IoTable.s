.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegStore16Pointer index
    .if (\index << 1) == 0x00
        .short emu_ioFallbackStore16
    .else
        .short emu_ioFallbackStore16
    .endif
.endm

.global memu_store16IoTable
memu_store16IoTable:
generate memu_ioRegStore16Pointer 0x106
