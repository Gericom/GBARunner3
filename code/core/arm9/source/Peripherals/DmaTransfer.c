#include "common.h"
#include <string.h>
#include <libtwl/gfx/gfxStatus.h>
#include "Emulator/IoRegisters.h"
#include "VirtualMachine/VMDtcm.h"
#include "MemCopy.h"
#include "GbaIoRegOffsets.h"
#include "SdCache/SdCache.h"
#include "MemoryEmulator/HiCodeCacheMapping.h"
#include "Peripherals/Sound/GbaSound9.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "GbaDma.h"
#include "MemoryEmulator/RomDefs.h"
#include "DmaTransfer.h"

DTCM_DATA dma_state_t dma_state;

void dma_immTransfer16(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep);
void dma_immTransfer32(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep);

extern void dma_immTransferSafe16(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep);
extern void dma_immTransferSafe16BadSrc(u32 dst, u32 byteCount, int dstStep);
extern void dma_immTransferSafe32(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep);
extern void dma_immTransferSafe32BadSrc(u32 dst, u32 byteCount, int dstStep);

extern u32 dma_transferRegister;
extern s8 dma_stepTable[4];

static inline void updateHBlankIrqForChannelStop(void)
{
    if (!(dma_state.dmaFlags & DMA_FLAG_HBLANK_MASK))
    {
        vm_forcedIrqMask &= ~(1 << 1);
        u32 gbaDispStat = *(u16*)&emu_ioRegisters[4];
        if (!(gbaDispStat & (1 << 4)))
        {
            gfx_setHBlankIrqEnabled(false);
        }
    }
}

static inline void updateArm7IrqForChannelStop(void)
{
    if (!(dma_state.dmaFlags & DMA_FLAG_SOUND_MASK))
    {
        vm_forcedIrqMask &= ~(1 << 16); // arm7 irq
    }
}

static inline void triggerDmaIrqIfEnabled(int channel, u32 control)
{
    if (control & GBA_DMA_CONTROL_IRQ)
    {
        vm_emulatedIfImeIe |= 1 << (8 + channel);
    }
}

static inline int getSrcStep(u32 control)
{
    return dma_stepTable[(control >> GBA_DMA_CONTROL_SRC_STEP_SHIFT) & GBA_DMA_CONTROL_SRC_STEP_MASK];
}

static inline int getDstStep(u32 control)
{
    return dma_stepTable[(control >> GBA_DMA_CONTROL_DST_STEP_SHIFT) & GBA_DMA_CONTROL_DST_STEP_MASK];
}

ITCM_CODE void dma_dmaTransfer(int channel)
{
    GbaDmaChannel* dmaIoBase = (GbaDmaChannel*)&emu_ioRegisters[GBA_REG_OFFS_DMA0SAD + channel * 0xC];
    u32 control = dmaIoBase->control;
    u32 count = dmaIoBase->count;
    if (count == 0)
        count = 0x10000;
    u32 byteCount;
    if (control & GBA_DMA_CONTROL_32BIT)
        byteCount = count << 2;
    else
        byteCount = count << 1;
    int srcStep = getSrcStep(control);
    u32 src = dma_state.channels[channel].curSrc;
    dma_state.channels[channel].curSrc += srcStep * byteCount;
    int dstStep = getDstStep(control);
    u32 dst;
    if (((control >> GBA_DMA_CONTROL_DST_STEP_SHIFT) & GBA_DMA_CONTROL_DST_STEP_MASK) != GBA_DMA_CONTROL_DST_STEP_RELOAD)
    {
        dst = dma_state.channels[channel].curDst;
        dma_state.channels[channel].curDst += dstStep * byteCount;
    }
    else
    {
        // reload
        dst = dmaIoBase->dst;
        dma_state.channels[channel].curDst = dst;
    }
    if (src >= 0x08000000 && src < 0x0E000000)
    {
        // rom always forces increment
        srcStep = 1;
    }
    if (control & GBA_DMA_CONTROL_32BIT)
        dma_immTransfer32(src, dst, byteCount, srcStep, dstStep);
    else
        dma_immTransfer16(src, dst, byteCount, srcStep, dstStep);
    triggerDmaIrqIfEnabled(channel, control);
    if (!(control & GBA_DMA_CONTROL_REPEAT))
    {
        dmaIoBase->control = control & ~GBA_DMA_CONTROL_ENABLED;
        dma_state.dmaFlags &= ~DMA_FLAG_HBLANK(channel);
        updateHBlankIrqForChannelStop();
    }
}

void dma_init(void)
{
    memset(&dma_state, 0, sizeof(dma_state));
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
            if (!(address & 0x10000))
            {
                break;
            }

            if (address & 0x4000)
            {
                address += 0x003F0000;
            }
            else
            {
                u32 dispCnt = emu_ioRegisters[GBA_REG_OFFS_DISPCNT];
                if ((dispCnt & 7) < 3 || address & 0x8000)
                {
                    address += 0x003F0000;
                }
            }
            break;
        }
    }
    return address;
}

ITCM_CODE static u32 dmaIoBaseToChannel(const GbaDmaChannel* dmaIoBase)
{
    u32 regOffset = (u32)dmaIoBase - (u32)emu_ioRegisters;
    return (regOffset - GBA_REG_OFFS_DMA0SAD) / (GBA_REG_OFFS_DMA1SAD - GBA_REG_OFFS_DMA0SAD);
}

static inline void dma_immTransferRomSrc(u32 src, u32 dst, u32 byteCount, bool dma32)
{
    u8* dstPtr = (u8*)dst;
    do
    {
        const void* cacheBlock = sdc_getRomBlock(src);
        u32 offset = src & SDC_BLOCK_MASK;
        u32 remainingInBlock = SDC_BLOCK_SIZE - offset;
        if (remainingInBlock > byteCount)
            remainingInBlock = byteCount;
        if (dma32)
            mem_copy32((const u8*)cacheBlock + offset, dstPtr, remainingInBlock);
        else
            mem_copy16((const u8*)cacheBlock + offset, dstPtr, remainingInBlock);
        src += remainingInBlock;
        dstPtr += remainingInBlock;
        byteCount -= remainingInBlock;
    } while (byteCount > 0);
}

static inline bool fastDmaSourceAllowed(u32 srcRegion)
{
    return 0b0011111111001100 & (1 << srcRegion);
}

static inline bool fastDmaDestinationAllowed(u32 dstRegion)
{
    u32 mask = 0b0000000011001100;
    if ((emu_ioRegisters[GBA_REG_OFFS_DISPCNT] & 7) >= 3)
    {
        mask &= ~(1 << 6);
    }

    return mask & (1 << dstRegion);
}

ITCM_CODE void dma_immTransfer16(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep)
{
    src &= ~1;
    dst &= ~1;
    if (src < 0x02000000)
    {
        dma_immTransferSafe16BadSrc(dst, byteCount, dstStep);
        return;
    }
    u32 srcRegion = src >> 24;
    u32 srcEnd = src + byteCount;
    u32 srcEndRegion = srcEnd >> 24;
    u32 dstRegion = dst >> 24;
    int difference = dst - src;
    if (difference < 0)
        difference = -difference;
    u32 dsDst = translateAddress(dst);
    u32 dsDstEnd = translateAddress(dst + (byteCount - 1) * dstStep);
    if (srcStep <= 0 || dsDstEnd != dsDst + (byteCount - 1) ||
        !fastDmaSourceAllowed(srcRegion) || !fastDmaDestinationAllowed(dstRegion) ||
        srcRegion != srcEndRegion || difference < 32)
    {
        dma_immTransferSafe16(src, dst, byteCount, srcStep, dstStep);
        return;
    }
    if (src >= 0x08000000)
    {
        dma_immTransferRomSrc(src, dsDst, byteCount, false);
    }
    else
    {
        src = translateAddress(src);
        mem_copy16((void*)src, (void*)dsDst, byteCount);
    }
    u32 last = ((u16*)dsDst)[(byteCount >> 1) - 1];
    dma_transferRegister = last | (last << 16);
}

ITCM_CODE void dma_immTransfer32(u32 src, u32 dst, u32 byteCount, int srcStep, int dstStep)
{
    src &= ~3;
    dst &= ~3;
    if (src < 0x02000000)
    {
        dma_immTransferSafe32BadSrc(dst, byteCount, dstStep);
        return;
    }
    u32 srcRegion = src >> 24;
    u32 srcEnd = src + byteCount;
    u32 srcEndRegion = srcEnd >> 24;
    u32 dstRegion = dst >> 24;
    int difference = dst - src;
    if (difference < 0)
        difference = -difference;
    u32 dsDst = translateAddress(dst);
    u32 dsDstEnd = translateAddress(dst + (byteCount - 1) * dstStep);
    if (srcStep <= 0 || dsDstEnd != dsDst + (byteCount - 1) ||
        !fastDmaSourceAllowed(srcRegion) || !fastDmaDestinationAllowed(dstRegion) ||
        srcRegion != srcEndRegion || difference < 32)
    {
        dma_immTransferSafe32(src, dst, byteCount, srcStep, dstStep);
        return;
    }
    if (src >= 0x08000000)
    {
        dma_immTransferRomSrc(src, dsDst, byteCount, true);
    }
    else
    {
        src = translateAddress(src);
        mem_copy32((void*)src, (void*)dsDst, byteCount);
    }
    dma_transferRegister = ((u32*)dsDst)[(byteCount >> 2) - 1];
}

ITCM_CODE static void dmaStop(int channel, GbaDmaChannel* dmaIoBase)
{
    dma_state.dmaFlags &= ~DMA_FLAG_HBLANK(channel);
    dma_state.dmaFlags &= ~DMA_FLAG_SOUND(channel);
    updateHBlankIrqForChannelStop();
    updateArm7IrqForChannelStop();
}

ITCM_CODE static void dmaStartHBlank(int channel, GbaDmaChannel* dmaIoBase, u32 value)
{
    u32 src = dmaIoBase->src;
    if ((src >= ROM_LINEAR_DS_ADDRESS && src < ROM_LINEAR_END_DS_ADDRESS))
        return;
    dmaIoBase->control = value;
    dma_state.dmaFlags |= DMA_FLAG_HBLANK(channel);
    vm_forcedIrqMask |= 1 << 1; // hblank irq
    gfx_setHBlankIrqEnabled(true);
    dma_state.channels[channel].curSrc = src;
    dma_state.channels[channel].curDst = dmaIoBase->dst;
}

ITCM_CODE void dma_dmaSound(u32 channel)
{
    dc_drainWriteBuffer();

    gbas_direct_channel_t* directChannel;
    GbaDmaChannel* dmaIoBase;
    if (channel == 1)
    {
        directChannel = &gGbaSoundShared.directChannels[0];
        dmaIoBase = (GbaDmaChannel*)&emu_ioRegisters[GBA_REG_OFFS_DMA1SAD];
    }
    else
    {
        directChannel = &gGbaSoundShared.directChannels[1];
        dmaIoBase = (GbaDmaChannel*)&emu_ioRegisters[GBA_REG_OFFS_DMA2SAD];
    }

    dc_invalidateLine(&directChannel->dmaRequest);
    if (!directChannel->dmaRequest)
        return;

    u32 control = dmaIoBase->control;
    u32 src = dma_state.channels[channel].curSrc;
    int srcStep = getSrcStep(control);
    if (src >= 0x02000000)
    {
        dma_state.channels[channel].curSrc += srcStep * 16;
        u32 dst = dma_state.channels[channel].curDst;
        dma_immTransferSafe32(src, dst, 16, srcStep, 0);
    }

    directChannel->dmaRequest = false;
    dc_drainWriteBuffer();

    triggerDmaIrqIfEnabled(channel, control);
    if (!(control & GBA_DMA_CONTROL_REPEAT))
    {
        dmaIoBase->control = control & ~GBA_DMA_CONTROL_ENABLED;
        dma_state.dmaFlags &= ~DMA_FLAG_SOUND(channel);
        updateArm7IrqForChannelStop();
    }
}

ITCM_CODE static void dmaStartSound(int channel, GbaDmaChannel* dmaIoBase, u32 value)
{
    dmaIoBase->control = value;
    dma_state.dmaFlags |= DMA_FLAG_SOUND(channel);
    vm_forcedIrqMask |= 1 << 16; // arm7 irq
    dma_state.channels[channel].curSrc = dmaIoBase->src;
    dma_state.channels[channel].curDst = dmaIoBase->dst;
    gGbaSoundShared.directChannels[channel - 1].dmaRequest = false;
    dc_drainWriteBuffer();
}

ITCM_CODE static void dmaStartSpecial(int channel, GbaDmaChannel* dmaIoBase, u32 value)
{
    u32 src = dmaIoBase->src;
    if ((src >= ROM_LINEAR_DS_ADDRESS && src < ROM_LINEAR_END_DS_ADDRESS))
        return;
    switch (channel)
    {
        case 0:
            break;
        case 1:
        case 2:
            dmaStartSound(channel, dmaIoBase, value);
            break;
        case 3:
            break;
    }
}

ITCM_CODE static void dmaStartImmediate(int channel, GbaDmaChannel* dmaIoBase, u32 control)
{
    u32 count = dmaIoBase->count;
    if (count == 0)
        count = 0x10000;
    u32 src = dmaIoBase->src;
    if (src >= ROM_LINEAR_DS_ADDRESS && src < ROM_LINEAR_END_DS_ADDRESS)
    {
        // assume this is a pc-relative rom address
        src = src + ROM_LINEAR_GBA_ADDRESS - ROM_LINEAR_DS_ADDRESS;
    }
    u32 dst = dmaIoBase->dst;
    triggerDmaIrqIfEnabled(channel, control);
    if (channel == 3)
    {
        vm_enableNestedIrqs();
    }
    int srcStep = getSrcStep(control);
    if (src >= 0x08000000 && src < 0x0E000000)
    {
        // rom always forces increment
        srcStep = 1;
    }
    int dstStep = getDstStep(control);
    if (control & GBA_DMA_CONTROL_32BIT)
    {
        sdc_setIrqForbiddenReplacementRange((u32)src, count << 2);
        dma_immTransfer32(src, dst, count << 2, srcStep, dstStep);
    }
    else
    {
        sdc_setIrqForbiddenReplacementRange((u32)src, count << 1);
        dma_immTransfer16(src, dst, count << 1, srcStep, dstStep);
    }
    sdc_resetIrqForbiddenReplacementRange();
    if (channel == 3)
    {
        vm_disableNestedIrqs();
    }
}

ITCM_CODE static void dmaStart(int channel, GbaDmaChannel* dmaIoBase, u32 control)
{
    dmaIoBase->control = control & ~GBA_DMA_CONTROL_ENABLED;
    if (control & GBA_DMA_CONTROL_ROM_DREQ)
        return; // rom dreq
    switch ((control >> GBA_DMA_CONTROL_MODE_SHIFT) & GBA_DMA_CONTROL_MODE_MASK)
    {
        case GBA_DMA_CONTROL_MODE_IMMEDIATE:
        {
            dmaStartImmediate(channel, dmaIoBase, control);
            break;
        }
        case GBA_DMA_CONTROL_MODE_VBLANK:
        {
            // todo
            break;
        }
        case GBA_DMA_CONTROL_MODE_HBLANK:
        {
            dmaStartHBlank(channel, dmaIoBase, control);
            break;
        }
        case GBA_DMA_CONTROL_MODE_SPECIAL:
        {
            dmaStartSpecial(channel, dmaIoBase, control);
            break;
        }
    }
}

ITCM_CODE void dma_CntHStore16(GbaDmaChannel* dmaIoBase, u32 value)
{
#ifdef GBAR3_HICODE_CACHE_MAPPING
    hic_unmapRomBlock();
#endif
    ic_invalidateAll();
    int channel = dmaIoBaseToChannel(dmaIoBase);
    u32 oldCnt = dmaIoBase->control;
    if (!((oldCnt ^ value) & GBA_DMA_CONTROL_ENABLED))
    {
        // no change in start/stop
        dmaIoBase->control = value;
    }
    else if (!(value & GBA_DMA_CONTROL_ENABLED))
    {
        // dma was stopped
        dmaIoBase->control = value;
        dmaStop(channel, dmaIoBase);
    }
    else
    {
        // dma was started
        dmaStart(channel, dmaIoBase, value);
    }
}
