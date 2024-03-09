#pragma once
#include "SdCacheDefs.h"
#include "MemoryEmulator/HiCodeCacheMapping.h"
#include "cp15.h"

/// @brief The sd cache blocks.
extern u8 sdc_cache[SDC_BLOCK_COUNT][SDC_BLOCK_SIZE];

/// @brief Maps rom blocks to their location in memory.
///        A value of 0 indicates the block is not loaded.
///        This table includes pointers to the linearly loaded part of the rom.
extern void* sdc_romBlockToCacheBlock[SDC_ROM_BLOCK_COUNT];

extern vu32 gSdCacheIrqForbiddenRomBlockReplacementRange;

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Loads a rom block into the cache. This method
///        assumes the block is not already in the cache.
/// @param romAddress An address in the rom to load to the cache.
/// @return A pointer to the newly loaded block.
const void* sdc_loadRomBlockDirect(u32 romAddress);

/// @brief Permanently loads the rom block that contains the given romAddress
///        into the sd cache for the purpose of applying patches.
/// @param romAddress An address in the block to load permanently into the cache.
/// @return A pointer to romAddress in the cache block.
void* sdc_loadRomBlockForPatching(u32 romAddress);

static inline const void* sdc_getRomBlock(u32 romAddress)
{
    u32 romBlock = ((romAddress << 7) >> 7) >> SDC_BLOCK_SHIFT;
    void* data = sdc_romBlockToCacheBlock[romBlock];
    if (data)
        return data;
// #ifdef GBAR3_HICODE_CACHE_MAPPING
//     hic_unmapRomBlock();
// #endif
//     ic_invalidateAll();
    return sdc_loadRomBlockDirect(romAddress);
}

static inline void sdc_resetIrqForbiddenReplacementRange(void)
{
    gSdCacheIrqForbiddenRomBlockReplacementRange = 0;
}

static inline void sdc_setIrqForbiddenReplacementRange(u32 romAddress, u32 length)
{
    if (romAddress < 0x08000000 || romAddress >= 0x0E000000)
    {
        sdc_resetIrqForbiddenReplacementRange();
        return;
    }
    romAddress = (romAddress << 7) >> 7;
    u32 startRomBlock = romAddress >> SDC_BLOCK_SHIFT;
    u32 endRomBlock = (romAddress + length + (SDC_BLOCK_SIZE - 1)) >> SDC_BLOCK_SHIFT;
    gSdCacheIrqForbiddenRomBlockReplacementRange = startRomBlock | (endRomBlock << 16);
}

/// @brief Initializes the sd cache.
void sdc_init(void);

#ifdef __cplusplus
}
#endif
