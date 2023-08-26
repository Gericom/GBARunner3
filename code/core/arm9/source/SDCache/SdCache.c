#include "common.h"
#include "Fat/ff.h"
#include "Fat/diskio.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "SdCache.h"

void* sdc_romBlockToCacheBlock[SDC_ROM_BLOCK_COUNT];

/// @brief Random generator state for random cache replacement.
static u32 sRandomState;

/// @brief Maps sd cache blocks to rom blocks.
static u16 sCacheBlockToRomBlock[SDC_BLOCK_COUNT];

static DWORD sClusterTable[512];

// temporarily
extern FIL gFile;

/// @brief Returns a cache block to replace.
/// @return The index of the cache block to replace.
static u32 getBlockToReplace(void)
{
    sRandomState = sRandomState * 1566083941u + 2531011u;
    return ((sRandomState >> 16) * SDC_BLOCK_COUNT) >> 16;
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
}

extern void logAddress(u32 address);

const void* sdc_loadRomBlockDirect(u32 romAddress)
{
    vm_enableNestedIrqs();
    ic_invalidateAll();
    // logAddress(romAddress);
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    u32 blockIdx = getBlockToReplace();
    u32 oldRomBlock = sCacheBlockToRomBlock[blockIdx];
    if (oldRomBlock != SDC_ROM_BLOCK_INVALID)
    {
        sdc_romBlockToCacheBlock[oldRomBlock] = 0;
    }
    sCacheBlockToRomBlock[blockIdx] = romBlock;
    void* cacheBlock = &sdc_cache[blockIdx][0];
    sdc_romBlockToCacheBlock[romBlock] = cacheBlock;
    loadRomBlock(romBlock, cacheBlock);
    vm_disableNestedIrqs();
    return cacheBlock;
}

void sdc_init(void)
{
    sRandomState = 0xA512ED48; // initial random seed
    for (u32 i = 0; i < SDC_ROM_BLOCK_COUNT; i++)
    {
        u32 romAddress = i * SDC_BLOCK_SIZE;
        if (romAddress < 2 * 1024 * 1024)
        {
            sdc_romBlockToCacheBlock[i] = (void*)(0x02200000 + romAddress);
        }
        else
        {
            sdc_romBlockToCacheBlock[i] = 0;
        }
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
