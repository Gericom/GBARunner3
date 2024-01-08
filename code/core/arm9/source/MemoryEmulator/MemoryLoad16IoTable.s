.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

.macro memu_ioRegLoad16Pointer index
    .if (\index << 1) == GBA_REG_OFFS_DISPCNT
        .short emu_ioFallbackLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_DISPSTAT
        .short emu_regDispStatLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_VCOUNT
        .short emu_regVCountLoad16
    .elseif ((\index << 1) >= GBA_REG_OFFS_BG0HOFS) && ((\index << 1) <= GBA_REG_OFFS_WIN1V)
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_MOSAIC
        .short memu_load16Undefined
    .elseif (\index << 1) == 0x4E
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_BLDY
        .short memu_load16Undefined
    .elseif ((\index << 1) > GBA_REG_OFFS_BLDY) && ((\index << 1) < GBA_REG_OFFS_SOUND1CNT_L)
        .short memu_load16Undefined
    .elseif (\index << 1) == 0x66
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x6A
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x6E
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x76
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x7A
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x7E
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == GBA_REG_OFFS_SOUNDCNT_X
        .short emu_gbaSoundLoadSoundCntX
    .elseif (\index << 1) == 0x86
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x8A
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x8C
        .short memu_load16Undefined
    .elseif (\index << 1) == 0x8E
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_A_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_A_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_B_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_FIFO_B_H
        .short memu_load16Undefined
    .elseif (\index << 1) == 0xA8
        .short memu_load16Undefined
    .elseif (\index << 1) == 0xAA
        .short memu_load16Undefined
    .elseif (\index << 1) == 0xAC
        .short memu_load16Undefined
    .elseif (\index << 1) == 0xAE
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0SAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0SAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0DAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0DAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA0CNT_L
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1SAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1SAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1DAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1DAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA1CNT_L
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2SAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2SAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2DAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2DAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA2CNT_L
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3SAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3SAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3DAD_L
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3DAD_H
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_DMA3CNT_L
        .short memu_load16UndefinedZero
    .elseif ((\index << 1) > GBA_REG_OFFS_DMA3CNT_H) && ((\index << 1) < GBA_REG_OFFS_TM0CNT_L)
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_TM0CNT_L
        .short emu_timerCntLLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_TM1CNT_L
        .short emu_timerCntLLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_TM2CNT_L
        .short emu_timerCntLLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_TM3CNT_L
        .short emu_timerCntLLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_SIOCNT
        .short emu_regSioCntLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_KEYINPUT
        .short emu_regKeyInputLoad16
    .elseif (\index << 1) == 0x136
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x142
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == 0x15A
        .short memu_load16UndefinedZero
    .elseif ((\index << 1) >= 0x15C) && ((\index << 1) < GBA_REG_OFFS_IE)
        .short memu_load16Undefined
    .elseif (\index << 1) == GBA_REG_OFFS_IE
        .short emu_ioFallbackLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_IF
        .short emu_regIfLoad16
    .elseif (\index << 1) == GBA_REG_OFFS_WAITCNT
        .short emu_ioFallbackLoad16
    .elseif (\index << 1) == 0x206
        .short memu_load16UndefinedZero
    .elseif (\index << 1) == GBA_REG_OFFS_IME
        .short emu_ioFallbackLoad16
    .elseif (\index << 1) == 0x20A
        .short memu_load16UndefinedZero
    .else
        .short emu_ioFallbackLoad16
    .endif
.endm

.global memu_load16IoTable
memu_load16IoTable:
generate memu_ioRegLoad16Pointer 0x106
