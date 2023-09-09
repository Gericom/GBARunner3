#include "common.h"
#include <string.h>
#include "SdCache/SdCache.h"
#include "cp15.h"
#include "JitArm.h"
#include "JitThumb.h"
#include "JitCommon.h"

/// @brief Stores for each halfword in the statically loaded part of the rom
///        whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gStaticRomJitBits[(2 * 1024 * 1024) / 2 / 32];

/// @brief Stores for each halfword in the dynamically loaded part of the rom (sd cache)
///        whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gDynamicRomJitBits[SDC_SIZE / 2 / 32];

/// @brief Stores for each halfword in IWRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gIWramJitBits[(32 * 1024) / 2 / 32];

/// @brief Stores for each halfword in EWRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gEWramJitBits[(256 * 1024) / 2 / 32];

/// @brief Stores for each halfword in VRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gVramJitBits[(96 * 1024) / 2 / 32];

[[gnu::section(".itcm")]]
u32* jit_getJitBits(const void* ptr)
{
    if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return &gStaticRomJitBits[((u32)ptr - 0x02200000) / 2 / 32];
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x03008000)
    {
        // IWRAM
        return &gIWramJitBits[((u32)ptr - 0x03000000) / 2 / 32];
    }
    else if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return &gDynamicRomJitBits[((u32)ptr - (u32)sdc_cache) / 2 / 32];
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        return &gEWramJitBits[((u32)ptr - 0x02000000) / 2 / 32];
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        return &gVramJitBits[((u32)ptr - 0x06000000) / 2 / 32];
    }
    return NULL;
}

void* jit_findBlockStart(const void* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (void*)((u32)ptr & ~SDC_BLOCK_MASK);
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (void*)0x02200000;
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
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (void*)0x02400000;
    }
    // no significant block boundary
    return (void*)0xFFFFFFFF;
}

[[gnu::section(".itcm")]]
bool jit_isBlockJitted(void* ptr)
{
    if ((u32)ptr >= 0x08000000)
    {
        ptr = (void*)((u32)ptr - 0x08000000 + 0x02200000);
    }

    const u32* const jitBits = jit_getJitBits(ptr);
    if (!jitBits)
        return true;
    u32 bitIdx = ((u32)ptr & 0x3F) >> 1;
    if (*jitBits & (1 << bitIdx))
        return true;

    return false;
}

[[gnu::section(".itcm")]]
void jit_ensureBlockJitted(void* ptr)
{
    if ((u32)ptr >= 0x08000000)
    {
        ptr = (void*)((u32)ptr - 0x08000000 + 0x02200000);
    }

    const u32* const jitBits = jit_getJitBits(ptr);
    if (!jitBits)
        return;
    u32 bitIdx = ((u32)ptr & 0x3F) >> 1;
    if (*jitBits & (1 << bitIdx))
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
    memset(gStaticRomJitBits, 0, sizeof(gStaticRomJitBits));
    memset(gDynamicRomJitBits, 0, sizeof(gDynamicRomJitBits));
    memset(gIWramJitBits, 0, sizeof(gIWramJitBits));
    memset(gEWramJitBits, 0, sizeof(gEWramJitBits));
    memset(gVramJitBits, 0, sizeof(gVramJitBits));
}
