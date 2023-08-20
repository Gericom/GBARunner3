#include "common.h"
#include <string.h>
#include <libtwl/gfx/gfxStatus.h>
#include "Emulator/IoRegisters.h"
#include "VirtualMachine/VMDtcm.h"
#include "MemCopy.h"
#include "DmaTransfer.h"

DTCM_DATA dma_state_t dma_state;

void dma_immTransfer16(u32 src, u32 dst, u32 count, int srcStep);
void dma_immTransfer32(u32 src, u32 dst, u32 count, int srcStep);

extern void dma_immTransferSafe16(u32 src, u32 dst, u32 count, int srcStep);
extern void dma_immTransferSafe32(u32 src, u32 dst, u32 count, int srcStep);

ITCM_CODE static void dmaDummy(void)
{
}

ITCM_CODE static inline void dmaTransfer(int channel, bool dma32)
{
    void* dmaIoBase = &emu_ioRegisters[0xB0 + channel * 0xC];
    u32 count = *(u16*)((u32)dmaIoBase + 8);
    if (channel == 3 && count == 0)
        count = 0x10000;
    u32 src = dma_state.channels[channel].curSrc;
    dma_state.channels[channel].curSrc += count << (dma32 ? 2 : 1);
    u32 dst = dma_state.channels[channel].curDst;
    u32 control = *(u16*)((u32)dmaIoBase + 0xA);
    if (((control >> 5) & 7) != 3)
    {
        dma_state.channels[channel].curDst += count << (dma32 ? 2 : 1);
    }
    else
    {
        // reload
        dma_state.channels[channel].curDst = *(u32*)((u32)dmaIoBase + 4);
    }
    if (dma32)
        dma_immTransfer32(src, dst, count, 1);//srcStep);
    else
        dma_immTransfer16(src, dst, count, 1);//srcStep);
}

ITCM_CODE static void dma0Transfer16(void)
{
    dmaTransfer(0, false);
}

ITCM_CODE static void dma1Transfer16(void)
{
    dmaTransfer(1, false);
}

ITCM_CODE static void dma2Transfer16(void)
{
    dmaTransfer(2, false);
}

ITCM_CODE static void dma3Transfer16(void)
{
    dmaTransfer(3, false);
}

ITCM_CODE static void dma0Transfer32(void)
{
    dmaTransfer(0, true);
}

ITCM_CODE static void dma1Transfer32(void)
{
    dmaTransfer(1, true);
}

ITCM_CODE static void dma2Transfer32(void)
{
    dmaTransfer(2, true);
}

ITCM_CODE static void dma3Transfer32(void)
{
    dmaTransfer(3, true);
}

void dma_init(void)
{
    memset(&dma_state, 0, sizeof(dma_state));
    dma_state.channels[0].dmaFunction = (void*)dmaDummy;
    dma_state.channels[1].dmaFunction = (void*)dmaDummy;
    dma_state.channels[2].dmaFunction = (void*)dmaDummy;
    dma_state.channels[3].dmaFunction = (void*)dmaDummy;
}

ITCM_CODE static u32 translateAddress(u32 address)
{
    switch (address >> 24)
    {
        case 2:
            address &= ~0x00FC0000;
            break;
        case 3:
            address &= ~0x00FF8000;
            break;
        case 5:
        case 7:
            address &= ~0x00FFFC00;
            break;
        case 6:
        {
            address &= ~0x00FE0000;
            if (address >= 0x06018000)
                address &= ~0x8000;
            
            u32 dispCnt = emu_ioRegisters[0]; // GBA REG_DISPCNT
            u32 objVramStart;
            if ((dispCnt & 7) < 3)
                objVramStart = 0x06010000;
            else
                objVramStart = 0x06014000;

            if (address >= objVramStart)
                address += 0x3F0000;
            break;
        }
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
            address &= ~0x06000000;
            address += 0x02200000 - 0x08000000;
            break;
    }
    return address;
}

ITCM_CODE static u32 dmaIoBaseToChannel(const void* dmaIoBase)
{
    u32 regOffset = (u32)dmaIoBase - (u32)emu_ioRegisters;
    switch (regOffset)
    {
        case 0xB0:
            return 0;
        case 0xBC:
            return 1;
        case 0xC8:
            return 2;
        case 0xD4:
            return 3;
    }
    // not possible
    return 0;
}

ITCM_CODE void dma_immTransfer16(u32 src, u32 dst, u32 count, int srcStep)
{
    src &= ~1;
    dst &= ~1;
    u32 srcEnd = src + (srcStep << 1) * count;
    u32 dstEnd = dst + (count << 1);
    u32 srcRegion = src >> 24;
    u32 srcEndRegion = srcEnd >> 24;
    u32 dstRegion = dst >> 24;
    u32 dstEndRegion = dstEnd >> 24;
    int difference = dst - src;
    if (difference < 0)
        difference = -difference;
    if (srcRegion == 4 || dstRegion == 4 ||
        srcRegion != srcEndRegion || dstRegion != dstEndRegion ||
        srcStep <= 0 || dstRegion >= 8 || difference < 32)
    {
        dma_immTransferSafe16(src, dst, count, srcStep);
        return;
    }
    // todo: check for bg vram -> obj vram transition
    src = translateAddress(src);
    dst = translateAddress(dst);
    mem_copy16((void*)src, (void*)dst, count << 1);
}

ITCM_CODE void dma_immTransfer32(u32 src, u32 dst, u32 count, int srcStep)
{
    src &= ~3;
    dst &= ~3;
    u32 srcEnd = src + (srcStep << 2) * count;
    u32 dstEnd = dst + (count << 2);
    u32 srcRegion = src >> 24;
    u32 srcEndRegion = srcEnd >> 24;
    u32 dstRegion = dst >> 24;
    u32 dstEndRegion = dstEnd >> 24;
    int difference = dst - src;
    if (difference < 0)
        difference = -difference;
    if (srcRegion == 4 || dstRegion == 4 ||
        srcRegion != srcEndRegion || dstRegion != dstEndRegion ||
        srcStep <= 0 || dstRegion >= 8 || difference < 32)
    {
        dma_immTransferSafe32(src, dst, count, srcStep);
        return;
    }
    // todo: check for bg vram -> obj vram transition
    src = translateAddress(src);
    dst = translateAddress(dst);
    mem_copy32((void*)src, (void*)dst, count << 2);
}

ITCM_CODE static void dmaStop(void* dmaIoBase, u32 value)
{
    *(u16*)(dmaIoBase + 0xA) = value;
    int channel = dmaIoBaseToChannel(dmaIoBase);
    dma_state.dmaFlags &= ~DMA_FLAG_HBLANK(channel);
    dma_state.channels[channel].dmaFunction = (void*)dmaDummy;
    if (!(dma_state.dmaFlags & 0xF))
    {
        vm_forcedIrqMask &= ~(1 << 1);
        u32 gbaDispStat = *(u16*)&emu_ioRegisters[4];
        if (!(gbaDispStat & (1 << 4)))
        {
            gfx_setHBlankIrqEnabled(false);
        }
    }
}

ITCM_CODE static void dmaStartHBlank(void* dmaIoBase, u32 value)
{
    *(u16*)(dmaIoBase + 0xA) = value;
    int channel = dmaIoBaseToChannel(dmaIoBase);
    dma_state.dmaFlags |= DMA_FLAG_HBLANK(channel);
    vm_forcedIrqMask |= 1 << 1; // hblank irq
    gfx_setHBlankIrqEnabled(true);
    dma_state.channels[channel].curSrc = *(u32*)dmaIoBase;
    dma_state.channels[channel].curDst = *(u32*)(dmaIoBase + 4);
    if (value & (1 << 10))
    {
        switch (channel)
        {
            case 0:
                dma_state.channels[channel].dmaFunction = (void*)dma0Transfer32;
                break;
            case 1:
                dma_state.channels[channel].dmaFunction = (void*)dma1Transfer32;
                break;
            case 2:
                dma_state.channels[channel].dmaFunction = (void*)dma2Transfer32;
                break;
            case 3:
                dma_state.channels[channel].dmaFunction = (void*)dma3Transfer32;
                break;
        }
    }
    else
    {
        switch (channel)
        {
            case 0:
                dma_state.channels[channel].dmaFunction = (void*)dma0Transfer16;
                break;
            case 1:
                dma_state.channels[channel].dmaFunction = (void*)dma1Transfer16;
                break;
            case 2:
                dma_state.channels[channel].dmaFunction = (void*)dma2Transfer16;
                break;
            case 3:
                dma_state.channels[channel].dmaFunction = (void*)dma3Transfer16;
                break;
        }        
    }
}

ITCM_CODE static void dmaStart(void* dmaIoBase, u32 value)
{
    *(u16*)(dmaIoBase + 0xA) = value & ~0x8000;
    if (value & (1 << 11))
        return; // rom dreq
    switch ((value >> 12) & 3)
    {
        case 0:
            break;
        case 1: // vblank
            return;
        case 2: // hblank
            dmaStartHBlank(dmaIoBase, value);
            return;
        case 3: // special
            return;
    }
    int srcStep = 1;
    switch ((value >> 7) & 3)
    {
        case 1:
            srcStep = -1;
            break;
        case 2:
            srcStep = 0;
            break;
    }
    u32 count = *(u16*)(dmaIoBase + 8);
    if (count == 0)
        count = 0x10000;
    u32 src = *(u32*)dmaIoBase;
    u32 dst = *(u32*)(dmaIoBase + 4);
    if (value & (1 << 10))
        dma_immTransfer32(src, dst, count, srcStep);
    else
        dma_immTransfer16(src, dst, count, srcStep);
    // todo: irq
}

ITCM_CODE void dma_CntHStore16(void* dmaIoBase, u32 value)
{
    u32 oldCnt = *(u16*)(dmaIoBase + 0xA);
    if (!((oldCnt ^ value) & 0x8000))
    {
        // no change in start/stop
        *(u16*)(dmaIoBase + 0xA) = value;
    }
    else if (!(value & 0x8000))
    {
        // dma was stopped
        dmaStop(dmaIoBase, value);
    }
    else
    {
        // dma was started
        dmaStart(dmaIoBase, value);
    }
}
