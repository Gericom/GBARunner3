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

/// @brief Stores 2 auxillary bits for each halfword in the statically loaded part of the rom.
[[gnu::section(".ewram.bss")]]
u32 gStaticRomJitAuxBits[(2 * 1024 * 1024) / 32];

/// @brief Stores for each halfword in the dynamically loaded part of the rom (sd cache)
///        whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gDynamicRomJitBits[SDC_SIZE / 2 / 32];

/// @brief Stores 2 auxillary bits for each halfword in the dynamically loaded part of the rom (sd cache).
[[gnu::section(".ewram.bss")]]
u32 gDynamicRomJitAuxBits[SDC_SIZE / 32];

/// @brief Stores for each halfword in IWRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gIWramJitBits[(32 * 1024) / 2 / 32];

/// @brief Stores 2 auxillary bits for each halfword in IWRAM.
[[gnu::section(".ewram.bss")]]
u32 gIWramJitAuxBits[(32 * 1024) / 32];

/// @brief Stores for each halfword in EWRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gEWramJitBits[(256 * 1024) / 2 / 32];

/// @brief Stores 2 auxillary bits for each halfword in EWRAM.
[[gnu::section(".ewram.bss")]]
u32 gEWramJitAuxBits[(256 * 1024) / 32];

/// @brief Stores for each halfword in VRAM whether it was processed by the JIT (1) or not (0).
[[gnu::section(".ewram.bss")]]
u32 gVramJitBits[(96 * 1024) / 2 / 32];

/// @brief Stores  2 auxillary bits for each halfword in VRAM.
[[gnu::section(".ewram.bss")]]
u32 gVramJitAuxBits[(96 * 1024) / 32];

[[gnu::section(".itcm"), gnu::optimize("Oz")]]
u32* jit_getJitBits(const void* ptr)
{
    u32* jitBits;
    u32 offset;

    if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        jitBits = gStaticRomJitBits;
        offset = (u32)ptr - 0x02200000;
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x03008000)
    {
        // IWRAM
        jitBits = gIWramJitBits;
        offset = (u32)ptr - 0x03000000;
    }
    else if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        jitBits = gDynamicRomJitBits;
        offset = (u32)ptr - (u32)sdc_cache;
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        jitBits = gEWramJitBits;
        offset = (u32)ptr - 0x02000000;
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        jitBits = gVramJitBits;
        offset = (u32)ptr - 0x06000000;
    }
    else
    {
        jitBits = NULL;
        offset = 0;
    }

    return &jitBits[offset / 2 / 32];
}

[[gnu::section(".itcm"), gnu::optimize("Oz")]]
u32* jit_getJitAuxBits(const void* ptr)
{
    u32* jitAuxBits;
    u32 offset;

    if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        jitAuxBits = gStaticRomJitAuxBits;
        offset = (u32)ptr - 0x02200000;
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x03008000)
    {
        // IWRAM
        jitAuxBits = gIWramJitAuxBits;
        offset = (u32)ptr - 0x03000000;
    }
    else if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        jitAuxBits = gDynamicRomJitAuxBits;
        offset = (u32)ptr - (u32)sdc_cache;
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        jitAuxBits = gEWramJitAuxBits;
        offset = (u32)ptr - 0x02000000;
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        jitAuxBits = gVramJitAuxBits;
        offset = (u32)ptr - 0x06000000;
    }
    else
    {
        jitAuxBits = NULL;
        offset = 0;
    }

    return &jitAuxBits[offset / 32];
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
    memset(gStaticRomJitAuxBits, 0, sizeof(gStaticRomJitAuxBits));
    memset(gDynamicRomJitBits, 0, sizeof(gDynamicRomJitBits));
    memset(gDynamicRomJitAuxBits, 0, sizeof(gDynamicRomJitAuxBits));
    memset(gIWramJitBits, 0, sizeof(gIWramJitBits));
    memset(gIWramJitAuxBits, 0, sizeof(gIWramJitAuxBits));
    memset(gEWramJitBits, 0, sizeof(gEWramJitBits));
    memset(gEWramJitAuxBits, 0, sizeof(gEWramJitAuxBits));
    memset(gVramJitBits, 0, sizeof(gVramJitBits));
    memset(gVramJitAuxBits, 0, sizeof(gVramJitAuxBits));
}
