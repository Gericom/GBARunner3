.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegLoad32Pointer index
    .if (\index << 1) == 0x00
        .short emu_ioFallbackLoad32
    .else
        .short emu_ioFallbackLoad32
    .endif
.endm

.global memu_load32IoTable
memu_load32IoTable:
generate memu_ioRegLoad32Pointer 0x84
