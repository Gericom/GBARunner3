.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

.macro memu_ioRegStore16Pointer index
    .if (\index << 1) == GBA_REG_OFFS_DISPCNT
        .short emu_regDispCntStore16
    .elseif (\index << 1) == GBA_REG_OFFS_GREEN_SWAP
        .short memu_store16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DISPSTAT
        .short emu_regDispStatStore16
    .elseif (\index << 1) == GBA_REG_OFFS_VCOUNT
        .short memu_store16Undefined // REG_VCOUNT is read-only
    .elseif (\index << 1) == GBA_REG_OFFS_BG0CNT
        .short emu_regBgCnt01Store16
    .elseif (\index << 1) == GBA_REG_OFFS_BG1CNT
        .short emu_regBgCnt01Store16
    .elseif (\index << 1) == GBA_REG_OFFS_BG2CNT
        .short emu_regBgCnt2Store16
    .elseif (\index << 1) == GBA_REG_OFFS_BG3CNT
        .short emu_regBgCnt3Store16
    .elseif (\index << 1) == GBA_REG_OFFS_WININ
        .short emu_regWinInOutStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WINOUT
        .short emu_regWinInOutStore16
    .elseif (\index << 1) == GBA_REG_OFFS_BLDCNT
        .short emu_regBldCntStore16
    .elseif (\index << 1) == GBA_REG_OFFS_BLDALPHA
        .short emu_regBldAlphaStore16
    // these are sound on the gba
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND1CNT_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND1CNT_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND1CNT_X
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == 0x66
        .short memu_store16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND2CNT_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == 0x6A
        .short memu_store16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND2CNT_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND3CNT_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND3CNT_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND3CNT_X
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND4CNT_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUND4CNT_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUNDCNT_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUNDCNT_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUNDCNT_X
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SOUNDBIAS
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM0_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM0_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM1_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM1_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM2_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM2_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM3_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAVE_RAM3_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_A_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_A_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_B_L
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_B_H
        .short emu_gbaSoundStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0SAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0SAD_H
        .short emu_dmaInternalMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0DAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0DAD_H
        .short emu_dmaInternalMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0CNT_L
        .short emu_dma012CntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0CNT_H
        .short emu_dmaCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1SAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1SAD_H
        .short emu_dmaAnyMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1DAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1DAD_H
        .short emu_dmaInternalMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1CNT_L
        .short emu_dma012CntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1CNT_H
        .short emu_dmaCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2SAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2SAD_H
        .short emu_dmaAnyMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2DAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2DAD_H
        .short emu_dmaInternalMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2CNT_L
        .short emu_dma012CntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2CNT_H
        .short emu_dmaCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3SAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3SAD_H
        .short emu_dmaAnyMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3DAD_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3DAD_H
        .short emu_dmaAnyMemoryAddressHiStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3CNT_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3CNT_H
        .short emu_dmaCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM0CNT_L
        .short emu_timerCntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM0CNT_H
        .short emu_timer0CntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM1CNT_L
        .short emu_timerCntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM1CNT_H
        .short emu_timerCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM2CNT_L
        .short emu_timerCntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM2CNT_H
        .short emu_timerCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM3CNT_L
        .short emu_timerCntLStore16
    .elseif (\index << 1) == GBA_REG_OFFS_TM3CNT_H
        .short emu_timerCntHStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIODATA32_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIODATA32_H
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIOMULTI2
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIOMULTI3
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIOCNT
        .short emu_regSioCntStore16
    .elseif (\index << 1) == GBA_REG_OFFS_SIOMULTI_SEND
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_KEYCNT
        .short emu_regKeyControlStore16
    .elseif (\index << 1) == GBA_REG_OFFS_RCNT
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOYCNT
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOY_RECV_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOY_RECV_H
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOY_TRANS_L
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOY_TRANS_H
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_JOYSTAT
        .short emu_ioStore16
    .elseif ((\index << 1) > GBA_REG_OFFS_JOYSTAT) && ((\index << 1) < GBA_REG_OFFS_IE)
        .short memu_store16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_IE
        .short emu_regIeIfStore32
    .elseif (\index << 1) == GBA_REG_OFFS_IF
        .short emu_regIfStore16
    .elseif (\index << 1) == GBA_REG_OFFS_WAITCNT
        .short emu_ioStore16
    .elseif (\index << 1) == GBA_REG_OFFS_IME
        .short emu_regImeStore16
    .else
        .short emu_ioFallbackStore16
    .endif
.endm

.global memu_store16IoTable
memu_store16IoTable:
generate memu_ioRegStore16Pointer 0x106
