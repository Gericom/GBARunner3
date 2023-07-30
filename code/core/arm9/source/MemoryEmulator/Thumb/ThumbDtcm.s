.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_pointer baseName, reg
    .short \baseName\reg
.endm
.macro memu_pointer_rd baseName, arg
    memu_pointer \baseName, %((\arg>>0)&7)
.endm

.global memu_thumbDispatchTable
memu_thumbDispatchTable:

.global memu_thumbRnRmTable
memu_thumbRnRmTable:

.macro memu_thumbGetRnRm_pointer2 rn, rm
    .word memu_thumbGetR\rn\()R\rm
.endm
.macro memu_thumbGetRnRm_pointer arg
    memu_thumbGetRnRm_pointer2 %((\arg>>0)&7), %((\arg>>3)&7)
.endm

generateBackwards memu_thumbGetRnRm_pointer, 64

.macro memu_instructionPointer2 a, b, rd, rd2
    .if (\a == 0b01010) && (\b == 0b00)
        .short memu_thumbStrRegR\rd2
    .elseif (\a == 0b01010) && (\b == 0b01)
        .short memu_thumbStrhRegR\rd2
    .elseif (\a == 0b01010) && (\b == 0b10)
        .short memu_thumbStrbRegR\rd2
    .elseif (\a == 0b01010) && (\b == 0b11)
        .short memu_thumbLdrsbRegR\rd2
    .elseif (\a == 0b01011) && (\b == 0b00)
        .short memu_thumbLdrRegR\rd2
    .elseif (\a == 0b01011) && (\b == 0b01)
        .short memu_thumbLdrhRegR\rd2
    .elseif (\a == 0b01011) && (\b == 0b10)
        .short memu_thumbLdrbRegR\rd2
    .elseif (\a == 0b01011) && (\b == 0b11)
        .short memu_thumbLdrshRegR\rd2
    .elseif \a == 0b01100
        .short memu_thumbStrImmR\rd2
    .elseif \a == 0b01101
        .short memu_thumbLdrImmR\rd2
    .elseif \a == 0b01110
        .short memu_thumbStrbImmR\rd2
    .elseif \a == 0b01111
        .short memu_thumbLdrbImmR\rd2
    .elseif \a == 0b10000
        .short memu_thumbStrhImmR\rd2
    .elseif \a == 0b10001
        .short memu_thumbLdrhImmR\rd2
    .elseif \a == 0b11000
        .short memu_thumbStmiaR\rd
    .elseif \a == 0b11001
        .short memu_thumbLdmiaR\rd
    .else
        .short 0
    .endif
.endm

.macro memu_instructionPointer index
    memu_instructionPointer2 %((\index>>3)&31), %((\index>>1)&3), %((\index>>0)&7), %((\index>>8)&7)
.endm

generate memu_instructionPointer, 0x800, 2

.end
