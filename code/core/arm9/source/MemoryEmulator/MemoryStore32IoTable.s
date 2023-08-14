.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.macro memu_ioRegStore32Pointer index
    .if (\index << 2) == 0x00
        .short emu_ioFallbackStore32
    .elseif (\index << 2) == 0x04
        .short emu_regDispStatStore16 // REG_DISPSTAT
    // these are sound on the gba
    .elseif (\index << 2) == 0x60
        .short memu_store32Undefined
    .elseif (\index << 2) == 0x64
        .short memu_store32Undefined
    .elseif (\index << 2) == 0x68
        .short memu_store32Undefined
    .elseif (\index << 2) == 0x6C
        .short memu_store32Undefined
    .elseif (\index << 2) == 0xB0
        .short emu_dmaInternalMemoryAddressStore32 // REG_DMA0SAD
    .elseif (\index << 2) == 0xB4
        .short emu_dmaInternalMemoryAddressStore32 // REG_DMA0DAD
    .elseif (\index << 2) == 0xB8
        .short emu_dma012CntStore32 // REG_DMA0CNT
    .elseif (\index << 2) == 0xBC
        .short emu_dmaAnyMemoryAddressStore32 // REG_DMA1SAD
    .elseif (\index << 2) == 0xC0
        .short emu_dmaInternalMemoryAddressStore32 // REG_DMA1DAD
    .elseif (\index << 2) == 0xC4
        .short emu_dma012CntStore32 // REG_DMA1CNT
    .elseif (\index << 2) == 0xC8
        .short emu_dmaAnyMemoryAddressStore32 // REG_DMA2SAD
    .elseif (\index << 2) == 0xCC
        .short emu_dmaInternalMemoryAddressStore32 // REG_DMA2DAD
    .elseif (\index << 2) == 0xD0
        .short emu_dma012CntStore32 // REG_DMA2CNT
    .elseif (\index << 2) == 0xD4
        .short emu_dmaAnyMemoryAddressStore32 // REG_DMA3SAD
    .elseif (\index << 2) == 0xD8
        .short emu_dmaAnyMemoryAddressStore32 // REG_DMA3DAD
    .elseif (\index << 2) == 0xDC
        .short emu_dma3CntStore32 // REG_DMA3CNT
    .elseif ((\index << 2) >= 0x180) && ((\index << 2) < 0x200)
        .short memu_store32Undefined
    .elseif (\index << 2) == 0x200
        .short emu_regIeIfStore32 // REG_IE and REG_IF
    .elseif (\index << 2) == 0x204
        .short memu_store32Undefined // REG_WAITCNT (bottom 16 bits)
    .elseif (\index << 2) == 0x208
        .short emu_regImeStore16 // REG_IME
    .else
        .short emu_ioFallbackStore32
    .endif
.endm

.global memu_store32IoTable
memu_store32IoTable:
generate memu_ioRegStore32Pointer 0x84
