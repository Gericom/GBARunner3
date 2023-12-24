.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

.macro memu_ioRegStore32Pointer index
    .if (\index << 2) == GBA_REG_OFFS_DISPCNT
        .short emu_regDispCntStore16
    .elseif (\index << 2) == GBA_REG_OFFS_DISPSTAT
        .short emu_regDispStatStore16
    .elseif (\index << 2) == GBA_REG_OFFS_BG0CNT
        .short emu_regBgCnt01Store32 // REG_BG0CNT and REG_BG1CNT
    .elseif (\index << 2) == GBA_REG_OFFS_BG2CNT
        .short emu_regBgCnt23Store32 // REG_BG2CNT and REG_BG3CNT
    .elseif (\index << 2) == GBA_REG_OFFS_WININ
        .short emu_regWinInOutStore32 // REG_WININ and REG_WINOUT
    .elseif (\index << 2) == GBA_REG_OFFS_BLDCNT
        .short emu_regBldCntBldAlphaStore32 // REG_BLDCNT REG_BLDALPHA
    // these are sound on the gba
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND1CNT_L
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND1CNT_X
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND2CNT_L
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND2CNT_H
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND3CNT_L
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND3CNT_X
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND4CNT_L
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUND4CNT_H
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUNDCNT_L
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUNDCNT_X
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SOUNDBIAS
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_WAVE_RAM0
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_WAVE_RAM1
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_WAVE_RAM2
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_WAVE_RAM3
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_FIFO_A
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_FIFO_B
        .short emu_gbaSoundStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA0SAD
        .short emu_dmaInternalMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA0DAD
        .short emu_dmaInternalMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA0CNT
        .short emu_dma012CntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA1SAD
        .short emu_dmaAnyMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA1DAD
        .short emu_dmaInternalMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA1CNT
        .short emu_dma012CntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA2SAD
        .short emu_dmaAnyMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA2DAD
        .short emu_dmaInternalMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA2CNT
        .short emu_dma012CntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA3SAD
        .short emu_dmaAnyMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA3DAD
        .short emu_dmaAnyMemoryAddressStore32
    .elseif (\index << 2) == GBA_REG_OFFS_DMA3CNT
        .short emu_dma3CntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_TM0CNT
        .short emu_timer0CntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_TM1CNT
        .short emu_timerCntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_TM2CNT
        .short emu_timerCntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_TM3CNT
        .short emu_timerCntStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SIODATA32
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SIOMULTI2
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_SIOCNT
        .short emu_regSioCntSioMultiSendStore32
    .elseif (\index << 1) == GBA_REG_OFFS_KEYINPUT
        .short emu_regKeyControlStore32
    .elseif (\index << 2) == GBA_REG_OFFS_RCNT
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_JOYCNT
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_JOY_RECV
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_JOY_TRANS
        .short emu_ioStore32
    .elseif (\index << 2) == GBA_REG_OFFS_JOYSTAT
        .short emu_ioStore32
    .elseif ((\index << 2) > GBA_REG_OFFS_JOYSTAT) && ((\index << 2) < GBA_REG_OFFS_IE)
        .short memu_store32Undefined
    .elseif (\index << 2) == GBA_REG_OFFS_IE
        .short emu_regIeIfStore32 // REG_IE and REG_IF
    .elseif (\index << 2) == GBA_REG_OFFS_WAITCNT
        .short emu_ioStore16
    .elseif (\index << 2) == GBA_REG_OFFS_IME
        .short emu_regImeStore16
    .else
        .short emu_ioFallbackStore32
    .endif
.endm

.global memu_store32IoTable
memu_store32IoTable:
generate memu_ioRegStore32Pointer 0x84
