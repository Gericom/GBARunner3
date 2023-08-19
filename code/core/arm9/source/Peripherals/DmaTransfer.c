#include "common.h"
#include <string.h>
#include "Emulator/IoRegisters.h"

extern void dma_immTransferSafe16(u32 src, u32 dst, u32 count, int srcStep);
extern void dma_immTransferSafe32(u32 src, u32 dst, u32 count, int srcStep);

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
    if (srcRegion == 4 || dstRegion == 4 ||
        srcRegion != srcEndRegion || dstRegion != dstEndRegion || srcStep <= 0)
    {
        dma_immTransferSafe16(src, dst, count, srcStep);
        return;
    }
    // todo: check for bg vram -> obj vram transition
    src = translateAddress(src);
    dst = translateAddress(dst);
    // todo: use something safer and faster here
    memcpy((void*)dst, (void*)src, count << 1);
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
    if (srcRegion == 4 || dstRegion == 4 ||
        srcRegion != srcEndRegion || dstRegion != dstEndRegion || srcStep <= 0)
    {
        dma_immTransferSafe32(src, dst, count, srcStep);
        return;
    }
    // todo: check for bg vram -> obj vram transition
    src = translateAddress(src);
    dst = translateAddress(dst);
    // todo: use something safer and faster here
    memcpy((void*)dst, (void*)src, count << 2);
}

ITCM_CODE void dma_CntHStore16(void* dmaIoBase, u32 value)
{
    u32 oldCnt = *(u16*)(dmaIoBase + 0xA);
    *(u16*)(dmaIoBase + 0xA) = value & ~0x8000;
    if (oldCnt & 0x8000 || !(value & 0x8000))
        return;
    if (value & (7 << 11))
        return;
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
}
