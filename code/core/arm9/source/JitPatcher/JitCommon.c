#include "common.h"
#include <string.h>
#include "SdCache/SdCache.h"
#include "cp15.h"
#include "MemoryEmulator/RomDefs.h"
#include "JitArm.h"
#include "JitThumb.h"
#include "JitCommon.h"

[[gnu::section(".ewram.bss")]]
jit_state_t gJitState;

[[gnu::section(".itcm"), gnu::optimize("Oz")]]
u32 jit_getJitBitsOffset(const void* ptr)
{
    u32 jitBitsOffset;
    u32 offset;

    if ((u32)ptr >= ROM_LINEAR_DS_ADDRESS && (u32)ptr < ROM_LINEAR_END_DS_ADDRESS)
    {
        // static rom region
        jitBitsOffset = offsetof(jit_state_t, staticRomJitBits);
        offset = (u32)ptr - ROM_LINEAR_DS_ADDRESS;
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x04000000)
    {
        // IWRAM
        jitBitsOffset = offsetof(jit_state_t, iWramJitBits);
        offset = (u32)ptr & 0x7FFF;
    }
    else if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        jitBitsOffset = offsetof(jit_state_t, dynamicRomJitBits);
        offset = (u32)ptr - (u32)sdc_cache;
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        jitBitsOffset = offsetof(jit_state_t, eWramJitBits);
        offset = (u32)ptr - 0x02000000;
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        jitBitsOffset = offsetof(jit_state_t, vramJitBits);
        offset = (u32)ptr - 0x06000000;
    }
    else
    {
        jitBitsOffset = offsetof(jit_state_t, dummyJitBits);
        offset = 0;
    }

    return jitBitsOffset + (offset / 2 / 8);
}

void* jit_findBlockStart(const void* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (void*)((u32)ptr & ~SDC_BLOCK_MASK);
    }
    else if ((u32)ptr >= ROM_LINEAR_DS_ADDRESS && (u32)ptr < ROM_LINEAR_END_DS_ADDRESS)
    {
        // static rom region
        return (void*)ROM_LINEAR_DS_ADDRESS;
    }
    // no significant block boundary
    return (void*)0;
}

void* jit_findBlockEnd(const void* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (void*)(((u32)ptr & ~SDC_BLOCK_MASK) + SDC_BLOCK_SIZE);
    }
    else if ((u32)ptr >= ROM_LINEAR_DS_ADDRESS && (u32)ptr < ROM_LINEAR_END_DS_ADDRESS)
    {
        // static rom region
        return (void*)ROM_LINEAR_END_DS_ADDRESS;
    }
    // no significant block boundary
    return (void*)0xFFFFFFFF;
}

[[gnu::section(".itcm")]]
bool jit_isBlockJitted(void* ptr)
{
    if ((u32)ptr >= ROM_LINEAR_GBA_ADDRESS)
    {
        ptr = (void*)((u32)ptr - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS);
    }

    const u8* const jitBits = jit_getJitBits(ptr);
    u32 bitIdx = ((u32)ptr & 0xF) >> 1;
    return (*jitBits >> bitIdx) & 1;
}

[[gnu::section(".itcm")]]
void jit_ensureBlockJitted(void* ptr)
{
    if ((u32)ptr >= ROM_LINEAR_GBA_ADDRESS)
    {
        ptr = (void*)((u32)ptr - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS);
    }

    const u8* const jitBits = jit_getJitBits(ptr);
    u32 bitIdx = ((u32)ptr & 0xF) >> 1;
    if ((*jitBits >> bitIdx) & 1)
        return;
    if ((u32)ptr & 1)
    {
        jit_processThumbBlock((u16*)((u32)ptr & ~1));
    }
    else
    {
        jit_processArmBlock((u32*)ptr);
    }
    dc_drainWriteBuffer();
    ic_invalidateAll();
}

void jit_init(void)
{
    memset(&gJitState, 0, sizeof(gJitState));
    gJitState.dummyJitBits = ~0u;
}

void jit_disable(void)
{
    memset(gJitState.staticRomJitBits, 0xFF, sizeof(gJitState.staticRomJitBits));
    memset(gJitState.dynamicRomJitBits, 0xFF, sizeof(gJitState.dynamicRomJitBits));
    memset(gJitState.iWramJitBits, 0xFF, sizeof(gJitState.iWramJitBits));
    memset(gJitState.eWramJitBits, 0xFF, sizeof(gJitState.eWramJitBits));
    memset(gJitState.vramJitBits, 0xFF, sizeof(gJitState.vramJitBits));
}
