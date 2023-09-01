#include "common.h"
#include <string.h>
#include "SdCache/SdCache.h"
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

u32* jit_getJitBits(const u32* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return &gDynamicRomJitBits[((u32)ptr - (u32)sdc_cache) / 2 / 32];
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return &gStaticRomJitBits[((u32)ptr - 0x02200000) / 2 / 32];
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        return &gEWramJitBits[((u32)ptr - 0x02000000) / 2 / 32];
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x03008000)
    {
        // EWRAM
        return &gIWramJitBits[((u32)ptr - 0x03000000) / 2 / 32];
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        return &gVramJitBits[((u32)ptr - 0x06000000) / 2 / 32];
    }
    return NULL;
}

u32* jit_findBlockStart(const u32* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (u32*)((u32)ptr & ~SDC_BLOCK_MASK);
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (u32*)0x02200000;
    }
    // no significant block boundary
    return (u32*)0;
}

u32* jit_findBlockEnd(const u32* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (u32*)(((u32)ptr & ~SDC_BLOCK_MASK) + SDC_BLOCK_SIZE);
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (u32*)0x02400000;
    }
    // no significant block boundary
    return (u32*)0xFFFFFFFF;
}

void jit_init(void)
{
    memset(gStaticRomJitBits, 0, sizeof(gStaticRomJitBits));
    memset(gDynamicRomJitBits, 0, sizeof(gDynamicRomJitBits));
    memset(gIWramJitBits, 0, sizeof(gIWramJitBits));
    memset(gEWramJitBits, 0, sizeof(gEWramJitBits));
    memset(gVramJitBits, 0, sizeof(gVramJitBits));
}
