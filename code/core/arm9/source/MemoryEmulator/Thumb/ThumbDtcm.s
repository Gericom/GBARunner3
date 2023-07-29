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

// 0x0000 - rnrm table (32 bit entries)

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

@ // 0x0480 - ldr pc

@ .space 0x80

@ // 0x0500 - str reg

@ .macro memu_thumbStrRegRd_pointer arg
@     memu_pointer_rd memu_thumbStrRegR, arg
@ .endm
@ generate memu_thumbStrRegRd_pointer, 16

@ // 0x0520 - strh reg

@ .macro memu_thumbStrhRegRd_pointer arg
@     memu_pointer_rd memu_thumbStrhRegR, arg
@ .endm
@ generate memu_thumbStrhRegRd_pointer, 16

@ // 0x0540 - strb reg

@ .macro memu_thumbStrbRegRd_pointer arg
@     memu_pointer_rd memu_thumbStrbRegR, arg
@ .endm
@ generate memu_thumbStrbRegRd_pointer, 16

@ // 0x0560 - ldrsb reg

@ .macro memu_thumbLdrsbRegRd_pointer arg
@     memu_pointer_rd memu_thumbLdrsbRegR, arg
@ .endm
@ generate memu_thumbLdrsbRegRd_pointer, 16

@ // 0x0580 - ldr reg

@ .macro memu_thumbLdrRegRd_pointer arg
@     memu_pointer_rd memu_thumbLdrRegR, arg
@ .endm
@ generate memu_thumbLdrRegRd_pointer, 16

@ // 0x05A0 - ldrh reg

@ .macro memu_thumbLdrhRegRd_pointer arg
@     memu_pointer_rd memu_thumbLdrhRegR, arg
@ .endm
@ generate memu_thumbLdrhRegRd_pointer, 16

@ // 0x05C0 - ldrb reg

@ .macro memu_thumbLdrbRegRd_pointer arg
@     memu_pointer_rd memu_thumbLdrbRegR, arg
@ .endm
@ generate memu_thumbLdrbRegRd_pointer, 16

@ // 0x05E0 - ldrsh reg

@ .macro memu_thumbLdrshRegRd_pointer arg
@     memu_pointer_rd memu_thumbLdrshRegR, arg
@ .endm
@ generate memu_thumbLdrshRegRd_pointer, 16

@ // 0x0600 - str imm

@ .macro memu_thumbStrImmRd_pointer arg
@     memu_pointer_rd memu_thumbStrImmR, arg
@ .endm
@ generate memu_thumbStrImmRd_pointer, 64

@ // 0x0680 - ldr imm

@ .macro memu_thumbLdrImmRd_pointer arg
@     memu_pointer_rd memu_thumbLdrImmR, arg
@ .endm
@ generate memu_thumbLdrImmRd_pointer, 64

@ // 0x0700 - strb imm

@ .macro memu_thumbStrbImmRd_pointer arg
@     memu_pointer_rd memu_thumbStrbImmR, arg
@ .endm
@ generate memu_thumbStrbImmRd_pointer, 64

@ // 0x0780 - ldrb imm

@ .macro memu_thumbLdrbImmRd_pointer arg
@     memu_pointer_rd memu_thumbLdrbImmR, arg
@ .endm
@ generate memu_thumbLdrbImmRd_pointer, 64

@ // 0x0800 - strh imm

@ .macro memu_thumbStrhImmRd_pointer arg
@     memu_pointer_rd memu_thumbStrhImmR, arg
@ .endm
@ generate memu_thumbStrhImmRd_pointer, 64

@ // 0x0880 - ldrh imm

@ .macro memu_thumbLdrhImmRd_pointer arg
@     memu_pointer_rd memu_thumbLdrhImmR, arg
@ .endm
@ generate memu_thumbLdrhImmRd_pointer, 64

@ // 0x0900 - str sp

@ .space 0x80

@ // 0x0980 - ldr sp

@ .space 0x80

@ // 0x0A00 - unused

@ .space 0x140

@ // 0x0B40 - push

@ .space 0x20

@ // 0x0B60 - unused

@ .space 0x60

@ // 0x0BC0 - pop

@ .space 0x20

@ // 0x0BE0 - unused

@ .space 0x20

@ // 0x0C00 - stmia

@ .macro memu_thumbStmiaRn_pointer arg
@     memu_pointer memu_thumbStmiaR, %((\arg>>3)&7)
@ .endm

@ generate memu_thumbStmiaRn_pointer, 64

@ // 0x0C80 - ldmia

@ .macro memu_thumbLdmiaRn_pointer arg
@     memu_pointer memu_thumbLdmiaR, %((\arg>>3)&7)
@ .endm

@ generate memu_thumbLdmiaRn_pointer, 64

// 0x0D00 - unused

.end
