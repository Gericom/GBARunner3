.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegStore16Pointer index
    .if (\index << 1) == 0x00
        .short emu_ioFallbackStore16
    .elseif (\index << 1) == 0x004
        .short emu_regDispStatStore16 // REG_DISPSTAT
    .elseif (\index << 1) == 0x006
        .short memu_store16Undefined // REG_VCOUNT is read-only
    .elseif (\index << 1) == 0xB0
        .short memu_store16Undefined // REG_DMA0SAD_L
    .elseif (\index << 1) == 0xB2
        .short memu_store16Undefined // REG_DMA0SAD_H
    .elseif (\index << 1) == 0xB4
        .short memu_store16Undefined // REG_DMA0DAD_L
    .elseif (\index << 1) == 0xB6
        .short memu_store16Undefined // REG_DMA0DAD_H
    .elseif (\index << 1) == 0xB8
        .short memu_store16Undefined // REG_DMA0CNT_L
    .elseif (\index << 1) == 0xBA
        .short memu_store16Undefined // REG_DMA0CNT_H
    .elseif (\index << 1) == 0xBC
        .short memu_store16Undefined // REG_DMA1SAD_L
    .elseif (\index << 1) == 0xBE
        .short memu_store16Undefined // REG_DMA1SAD_H
    .elseif (\index << 1) == 0xC0
        .short memu_store16Undefined // REG_DMA1DAD_L
    .elseif (\index << 1) == 0xC2
        .short memu_store16Undefined // REG_DMA1DAD_H
    .elseif (\index << 1) == 0xC4
        .short memu_store16Undefined // REG_DMA1CNT_L
    .elseif (\index << 1) == 0xC6
        .short memu_store16Undefined // REG_DMA1CNT_H
    .elseif (\index << 1) == 0xC8
        .short memu_store16Undefined // REG_DMA2SAD_L
    .elseif (\index << 1) == 0xCA
        .short memu_store16Undefined // REG_DMA2SAD_H
    .elseif (\index << 1) == 0xCC
        .short memu_store16Undefined // REG_DMA2DAD_L
    .elseif (\index << 1) == 0xCE
        .short memu_store16Undefined // REG_DMA2DAD_H
    .elseif (\index << 1) == 0xD0
        .short memu_store16Undefined // REG_DMA2CNT_L
    .elseif (\index << 1) == 0xD2
        .short memu_store16Undefined // REG_DMA2CNT_H
    .elseif (\index << 1) == 0xD4
        .short memu_store16Undefined // REG_DMA3SAD_L
    .elseif (\index << 1) == 0xD6
        .short memu_store16Undefined // REG_DMA3SAD_H
    .elseif (\index << 1) == 0xD8
        .short memu_store16Undefined // REG_DMA3DAD_L
    .elseif (\index << 1) == 0xDA
        .short memu_store16Undefined // REG_DMA3DAD_H
    .elseif (\index << 1) == 0xDC
        .short memu_store16Undefined // REG_DMA3CNT_L
    .elseif (\index << 1) == 0xDE
        .short memu_store16Undefined // REG_DMA3CNT_H
    .elseif (\index << 1) == 0x200
        .short emu_regIeStore16 // REG_IE
    .elseif (\index << 1) == 0x202
        .short emu_regIfStore16 // REG_IF
    .elseif (\index << 1) == 0x204
        .short memu_store16Undefined // REG_WAITCNT
    .elseif (\index << 1) == 0x208
        .short emu_regImeStore16 // REG_IME
    .else
        .short emu_ioFallbackStore16
    .endif
.endm

.global memu_store16IoTable
memu_store16IoTable:
generate memu_ioRegStore16Pointer 0x106
