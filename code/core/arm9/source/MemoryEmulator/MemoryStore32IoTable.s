.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegStore32Pointer index
    .if (\index << 1) == 0x00
        .short emu_ioFallbackStore32
    .else
        .short emu_ioFallbackStore32
    .endif
.endm

.global memu_store32IoTable
memu_store32IoTable:
generate memu_ioRegStore32Pointer 0x84
