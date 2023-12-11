#include "common.h"
#include "Fat/ff.h"
#include "Fat/diskio.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "cp15.h"
#include "SdCache.h"

[[gnu::section(".vramhi.bss")]]
void* sdc_romBlockToCacheBlock[SDC_ROM_BLOCK_COUNT];

/// @brief Random generator state for random cache replacement.
static u32 sRandomState;

/// @brief Maps sd cache blocks to rom blocks.
static u16 sCacheBlockToRomBlock[SDC_BLOCK_COUNT];

/// @brief The number of usable blocks in the cache. This can be less than the
///        total number of cache blocks when some blocks are permanently loaded.
static u32 sBlockCount;

static DWORD sClusterTable[512];

// temporarily
extern FIL gFile;

/// @brief Returns a cache block to replace.
/// @return The index of the cache block to replace.
static u32 getBlockToReplace(void)
{
    sRandomState = sRandomState * 1566083941u + 2531011u;
    return ((sRandomState >> 16) * sBlockCount) >> 16;
}

/// @brief Loads a rom block to the given buffer.
/// @param romBlock Rom block index to load.
/// @param dst The destination buffer.
static void loadRomBlock(u32 romBlock, void* dst)
{
    u32 ofs = romBlock * SDC_BLOCK_SIZE;
    if (ofs >= f_size(&gFile))
        return;
    // f_lseek(&gFile, romBlock * SDC_BLOCK_SIZE);
    // UINT br;
    // f_read(&gFile, dst, SDC_BLOCK_SIZE, &br);
    DWORD cl, ncl, *tbl;
    FATFS *fs = gFile.obj.fs;

    tbl = gFile.cltbl + 1;	/* Top of CLMT */
    u32 csect = (UINT)(ofs / 512 & (fs->csize - 1));
    u32 cshift = __builtin_ctz(fs->csize) + 9;
    cl = (DWORD)(ofs >> cshift);	/* Cluster order from top of the file */
    while(true)
    {
        ncl = *tbl++;			/* Number of cluters in the fragment */
        if (cl < ncl) break;	/* In this fragment? */
        cl -= ncl; tbl++;		/* Next fragment */
    }
    u32 cluster = cl + *tbl;	/* Return the cluster number */
    cluster -= 2;		/* Cluster number is origin from 2 */
    u32 sector = fs->database + fs->csize * cluster;
    sector += csect;
    disk_read(fs->pdrv, dst, sector, SDC_BLOCK_SIZE / 512); 
    dc_flushRange(dst, SDC_BLOCK_SIZE);
}

static void* loadRomBlockDirect(u32 romBlock, u32 blockIdx)
{
    u32 oldRomBlock = sCacheBlockToRomBlock[blockIdx];
    if (oldRomBlock != SDC_ROM_BLOCK_INVALID)
    {
        sdc_romBlockToCacheBlock[oldRomBlock] = 0;
    }
    sCacheBlockToRomBlock[blockIdx] = romBlock;
    void* cacheBlock = &sdc_cache[blockIdx][0];
    sdc_romBlockToCacheBlock[romBlock] = cacheBlock;
    loadRomBlock(romBlock, cacheBlock);
    return cacheBlock;
}

extern void logAddress(u32 address);

const void* sdc_loadRomBlockDirect(u32 romAddress)
{
    vm_enableNestedIrqs();
    // logAddress(romAddress);
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    u32 blockIdx = getBlockToReplace();
    void* cacheBlock = loadRomBlockDirect(romBlock, blockIdx);
    vm_disableNestedIrqs();
    return cacheBlock;
}

void* sdc_loadRomBlockForPatching(u32 romAddress)
{
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    void* data = sdc_romBlockToCacheBlock[romBlock];
    // if not loaded at all yet, or not permanent
    if (!data || (u32)data < &sdc_cache[sBlockCount][0])
    {
        if (data)
        {
            // if already loaded, but not permanent, invalidate block
            sCacheBlockToRomBlock[((u32)data - (u32)&sdc_cache[0][0]) / SDC_BLOCK_SIZE] = SDC_ROM_BLOCK_INVALID;
        }

        data = loadRomBlockDirect(romBlock, --sBlockCount);
    }
    return (void*)((u32)data + (romAddress & SDC_BLOCK_MASK));
}

void sdc_init(void)
{
    sRandomState = 0xA512ED48; // initial random seed
    sBlockCount = SDC_BLOCK_COUNT;
    for (u32 i = 0; i < SDC_ROM_BLOCK_COUNT; i++)
    {
        sdc_romBlockToCacheBlock[i] = NULL;
    }
    for (u32 i = 0; i < SDC_BLOCK_COUNT; i++)
        sCacheBlockToRomBlock[i] = SDC_ROM_BLOCK_INVALID;

    sClusterTable[0] = sizeof(sClusterTable) / sizeof(DWORD);
    gFile.cltbl = sClusterTable;
    u32 result = f_lseek(&gFile, CREATE_LINKMAP);
    if (result != FR_OK)
    {
        logAddress(0xDEADBEEF);
        logAddress(result);
    }
}
