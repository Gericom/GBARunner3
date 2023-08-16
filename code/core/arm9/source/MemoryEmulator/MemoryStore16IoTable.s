.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegStore16Pointer index
    .if (\index << 1) == 0x00
        .short emu_regDispCntStore16
    .elseif (\index << 1) == 0x004
        .short emu_regDispStatStore16 // REG_DISPSTAT
    .elseif (\index << 1) == 0x006
        .short memu_store16Undefined // REG_VCOUNT is read-only
    // these are sound on the gba
    .elseif (\index << 1) == 0x60
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x62
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x64
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x66
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x68
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x6A
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x6C
        .short memu_store16Undefined
    .elseif (\index << 1) == 0xB0
        .short emu_dmaStore16 // REG_DMA0SAD_L
    .elseif (\index << 1) == 0xB2
        .short emu_dmaInternalMemoryAddressHiStore16 // REG_DMA0SAD_H
    .elseif (\index << 1) == 0xB4
        .short emu_dmaStore16 // REG_DMA0DAD_L
    .elseif (\index << 1) == 0xB6
        .short emu_dmaInternalMemoryAddressHiStore16 // REG_DMA0DAD_H
    .elseif (\index << 1) == 0xB8
        .short emu_dma012CntLStore16 // REG_DMA0CNT_L
    .elseif (\index << 1) == 0xBA
        .short emu_dmaCntHStore16 // REG_DMA0CNT_H
    .elseif (\index << 1) == 0xBC
        .short emu_dmaStore16 // REG_DMA1SAD_L
    .elseif (\index << 1) == 0xBE
        .short emu_dmaAnyMemoryAddressHiStore16 // REG_DMA1SAD_H
    .elseif (\index << 1) == 0xC0
        .short emu_dmaStore16 // REG_DMA1DAD_L
    .elseif (\index << 1) == 0xC2
        .short emu_dmaInternalMemoryAddressHiStore16 // REG_DMA1DAD_H
    .elseif (\index << 1) == 0xC4
        .short emu_dma012CntLStore16 // REG_DMA1CNT_L
    .elseif (\index << 1) == 0xC6
        .short emu_dmaCntHStore16 // REG_DMA1CNT_H
    .elseif (\index << 1) == 0xC8
        .short emu_dmaStore16 // REG_DMA2SAD_L
    .elseif (\index << 1) == 0xCA
        .short emu_dmaAnyMemoryAddressHiStore16 // REG_DMA2SAD_H
    .elseif (\index << 1) == 0xCC
        .short emu_dmaStore16 // REG_DMA2DAD_L
    .elseif (\index << 1) == 0xCE
        .short emu_dmaInternalMemoryAddressHiStore16 // REG_DMA2DAD_H
    .elseif (\index << 1) == 0xD0
        .short emu_dma012CntLStore16 // REG_DMA2CNT_L
    .elseif (\index << 1) == 0xD2
        .short emu_dmaCntHStore16 // REG_DMA2CNT_H
    .elseif (\index << 1) == 0xD4
        .short emu_dmaStore16 // REG_DMA3SAD_L
    .elseif (\index << 1) == 0xD6
        .short emu_dmaAnyMemoryAddressHiStore16 // REG_DMA3SAD_H
    .elseif (\index << 1) == 0xD8
        .short emu_dmaStore16 // REG_DMA3DAD_L
    .elseif (\index << 1) == 0xDA
        .short emu_dmaAnyMemoryAddressHiStore16 // REG_DMA3DAD_H
    .elseif (\index << 1) == 0xDC
        .short emu_dmaStore16 // REG_DMA3CNT_L
    .elseif (\index << 1) == 0xDE
        .short emu_dmaCntHStore16 // REG_DMA3CNT_H
    .elseif ((\index << 2) >= 0x180) && ((\index << 2) < 0x200)
        .short memu_store16Undefined
    .elseif (\index << 1) == 0x200
        .short emu_regIeIfStore32 // REG_IE
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
