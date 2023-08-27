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

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Loads a rom block into the cache. This method
///        assumes the block is not already in the cache.
/// @param romAddress An address in the rom to load to the cache.
/// @return A pointer to the newly loaded block.
const void* sdc_loadRomBlockDirect(u32 romAddress);

static inline const void* sdc_getRomBlock(u32 romAddress)
{
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    void* data = sdc_romBlockToCacheBlock[romBlock];
    if (data)
        return data;
#ifdef GBAR3_HICODE_CACHE_MAPPING
    hic_unmapRomBlock();
#endif
    ic_invalidateAll();
    return sdc_loadRomBlockDirect(romAddress);
}

/// @brief Initializes the sd cache.
void sdc_init(void);

#ifdef __cplusplus
}
#endif
