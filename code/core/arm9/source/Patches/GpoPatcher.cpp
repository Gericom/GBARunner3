#include "GpoPatcher.h"
#include "GpoBuilder.h"
#include "UpsPatcher.h"
#include "MemoryEmulator/RomDefs.h"
#include <string.h>
#include <stdlib.h>

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
u32 gGpoBitmask[8192 / 32];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
FIL gGpoFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static DWORD sGpoClusterTable[512];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static DWORD sMergedClusterTable[2048];

// Static allocations in EWRAM BSS to prevent stack overflow in DTCM
[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FILINFO sFnoIps;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sIpsPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sUpsPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FILINFO sFnoUps;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sGpoPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sLogPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sActiveGpoHeader;

extern FIL gFile; // Holds the GBA ROM file object and its cltbl

struct ClmtTracker
{
    const DWORD* tbl;
    DWORD currentLogicalBase;
    DWORD currentFragSize;
    DWORD currentFragPhysStart;

    void Init(const DWORD* cltbl)
    {
        tbl = cltbl + 1;
        currentLogicalBase = 0;
        currentFragSize = *tbl++;
        currentFragPhysStart = *tbl++;
    }

    DWORD GetPhysicalCluster(DWORD logicalCluster)
    {
        while (currentFragSize > 0 && logicalCluster >= currentLogicalBase + currentFragSize)
        {
            currentLogicalBase += currentFragSize;
            currentFragSize = *tbl++;
            currentFragPhysStart = *tbl++;
        }
        if (currentFragSize == 0) return 0; // EOF / Error
        
        return currentFragPhysStart + (logicalCluster - currentLogicalBase);
    }
};

[[gnu::section(".ewram")]] static bool mergeClusterMaps(u32 romSize, u32 patchedRomSize, u32 clusterSize)
{
    u32 totalRomClusters     = (romSize        + clusterSize - 1) / clusterSize;
    u32 patchedTotalClusters = (patchedRomSize  + clusterSize - 1) / clusterSize;

    // patchedRomClusterCount: popcount of bitmask scoped to ROM region only.
    // Extension clusters do not set bitmask bits, so this must not count past totalRomClusters.
    u32 patchedRomClusterCount = 0;
    for (u32 c = 0; c < totalRomClusters; c++)
        if (c < 8192 && (gGpoBitmask[c / 32] & (1u << (c % 32))))
            patchedRomClusterCount++;

    DWORD* dest = sMergedClusterTable + 1;
    u32 tableLimit = 2048 - 2;

    u32 fragmentCount      = 0;
    u32 currentFragSize    = 0;
    DWORD currentFragStartPhys = 0;
    u32 patchedClusterIndex = 0;

    ClmtTracker romTracker;
    romTracker.Init(gFile.cltbl);

    ClmtTracker gpoTracker;
    gpoTracker.Init(sGpoClusterTable);

    for (u32 c = 0; c < patchedTotalClusters; c++)
    {
        DWORD physCl = 0;
        if (c >= totalRomClusters)
        {
            // Extension cluster: always from GPO, sequentially after all patched ROM clusters.
            // ClmtTracker monotone contract holds: indices are 1..P_rom then P_rom+1..P_rom+E.
            physCl = gpoTracker.GetPhysicalCluster(1 + patchedRomClusterCount + (c - totalRomClusters));
        }
        else
        {
            bool isPatched = c < 8192 && (gGpoBitmask[c / 32] & (1u << (c % 32)));
            if (isPatched)
            {
                physCl = gpoTracker.GetPhysicalCluster(1 + patchedClusterIndex);
                patchedClusterIndex++;
            }
            else
            {
                physCl = romTracker.GetPhysicalCluster(c);
            }
        }

        if (physCl == 0) return false;

        if (currentFragSize == 0)
        {
            currentFragSize = 1;
            currentFragStartPhys = physCl;
        }
        else if (physCl == currentFragStartPhys + currentFragSize)
        {
            currentFragSize++;
        }
        else
        {
            if ((u32)(dest - sMergedClusterTable) >= tableLimit) return false;
            *dest++ = currentFragSize;
            *dest++ = currentFragStartPhys;
            fragmentCount++;

            currentFragSize = 1;
            currentFragStartPhys = physCl;
        }
    }

    if (currentFragSize > 0)
    {
        if ((u32)(dest - sMergedClusterTable) >= tableLimit) return false;
        *dest++ = currentFragSize;
        *dest++ = currentFragStartPhys;
        fragmentCount++;
    }

    *dest = 0;
    sMergedClusterTable[0] = (dest - sMergedClusterTable);
    return true;
}

#ifndef NDEBUG
[[gnu::section(".ewram")]] static void gpo_runUnitTests(void)
{
    if (!gLogger) return;
    gLogger->Log(LogLevel::Debug, "[GPO Test] Running mergeClusterMaps unit tests...\n");

    DWORD* origCltbl = gFile.cltbl;

    // Mock ROM CLMT: 1000 clusters contiguous starting at physical cluster 100
    DWORD gbaCltblMock[4];
    gbaCltblMock[0] = 3;
    gbaCltblMock[1] = 1000;
    gbaCltblMock[2] = 100;
    gbaCltblMock[3] = 0;
    gFile.cltbl = gbaCltblMock;

    // Mock GPO CLMT: 2000 clusters contiguous starting at physical cluster 5000
    sGpoClusterTable[0] = 3;
    sGpoClusterTable[1] = 2000;
    sGpoClusterTable[2] = 5000;
    sGpoClusterTable[3] = 0;

    // Test 1: Merge under limit (200 ROM clusters, every-other patched, no extension)
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 200; i++)
        if (i % 2 == 1)
            gGpoBitmask[i / 32] |= (1u << (i % 32));
    bool res1 = mergeClusterMaps(200 * 4096, 200 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] 1 - Merge under limit (no extension): %s\n", res1 ? "PASS" : "FAIL");

    // Test 2: Merge over limit triggers overflow (alternating 1100 clusters ~= 1100 fragments, limit 1023)
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 1100; i++)
        if (i % 2 == 1)
            gGpoBitmask[i / 32] |= (1u << (i % 32));
    bool res2 = mergeClusterMaps(1100 * 4096, 1100 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] 2 - Merge overflow (expected false): %s\n", !res2 ? "PASS" : "FAIL");

    // Test 3: ROM extension -- 100 ROM clusters, 10 extension clusters (patchedRomSize = 110 clusters)
    // GPO CLMT has 2000 clusters; 100 patched ROM + 10 extension = 110 needed -- fits.
    // All 100 ROM clusters are patched (bitmask all 1s in [0..99]).
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 100; i++)
        gGpoBitmask[i / 32] |= (1u << (i % 32));
    bool res3 = mergeClusterMaps(100 * 4096, 110 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] 3 - ROM extension (100 ROM + 10 ext): %s\n", res3 ? "PASS" : "FAIL");

    // Test 4: ROM truncation -- 100 ROM clusters, patchedRomSize = 50 clusters. Merge stops at 50.
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 50; i++)
        gGpoBitmask[i / 32] |= (1u << (i % 32));
    bool res4 = mergeClusterMaps(100 * 4096, 50 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] 4 - ROM truncation (100 ROM -> 50): %s\n", res4 ? "PASS" : "FAIL");

    // Test 5: Unchanged size (patchedRomSize == romSize) -- identical to pre-extension behavior.
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 200; i++)
        if (i % 2 == 1)
            gGpoBitmask[i / 32] |= (1u << (i % 32));
    bool res5 = mergeClusterMaps(200 * 4096, 200 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] 5 - Unchanged size: %s\n", res5 ? "PASS" : "FAIL");

    gFile.cltbl = origCltbl;
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    gLogger->Log(LogLevel::Debug, "[GPO Test] Done.\n");
}
#endif

[[gnu::section(".ewram")]] static bool activateGpo(const char* gpoPath, u32 romSize)
{
    if (f_open(&gGpoFile, gpoPath, FA_READ | FA_OPEN_EXISTING) != FR_OK) return false;

    UINT br;
    if (f_read(&gGpoFile, &sActiveGpoHeader, sizeof(sActiveGpoHeader), &br) != FR_OK
        || br != sizeof(sActiveGpoHeader))
    {
        f_close(&gGpoFile);
        return false;
    }

    memcpy(gGpoBitmask, sActiveGpoHeader.bitmask, sizeof(gGpoBitmask));

    sGpoClusterTable[0] = sizeof(sGpoClusterTable) / sizeof(DWORD);
    gGpoFile.cltbl = sGpoClusterTable;
    if (f_lseek(&gGpoFile, CREATE_LINKMAP) != FR_OK)
    {
        f_close(&gGpoFile);
        return false;
    }

    u32 activeRomSize = sActiveGpoHeader.patchedRomSize > 0 ? sActiveGpoHeader.patchedRomSize : romSize;
    if (!mergeClusterMaps(romSize, activeRomSize, sActiveGpoHeader.clusterSize))
    {
        f_close(&gGpoFile);
        return false;
    }

    // Update SdCache bounds before swapping the CLMT; SdCache uses gFile.obj.objsize as
    // the upper bound for read validation. Must be set before gFile.cltbl so that the
    // first f_read after the swap sees the correct limit.
    gFile.obj.objsize = activeRomSize;
    gFile.cltbl = sMergedClusterTable;
    return true;
}

extern "C" [[gnu::section(".ewram")]] GpoInitResult gpo_init(const char* romPath)
{
    gpo_getPath(romPath, sUpsPath, ".ups");
    gpo_getPath(romPath, sIpsPath, ".ips");

    bool hasUps = f_stat(sUpsPath, &sFnoUps) == FR_OK;
    bool hasIps = f_stat(sIpsPath, &sFnoIps) == FR_OK;
    if (!hasUps && !hasIps) return GpoInitResult::Inactive;

#ifndef NDEBUG
    gpo_runUnitTests();
#endif

    FILINFO fnoRom;
    if (f_stat(romPath, &fnoRom) != FR_OK) return GpoInitResult::Inactive;
    u32 romSize = fnoRom.fsize;
    u32 clusterSize = gFile.obj.fs->csize * 512;

    gpo_getPath(romPath, sGpoPath, ".gpo");

#ifndef NDEBUG
    gpo_getPath(romPath, sLogPath, "_gpo.log");
    f_unlink(sLogPath);
    log_debug(romPath, "=== GPO INIT ===");
    log_debug(romPath, "ROM size loaded:", romSize, 0);
    log_debug(romPath, "Cluster size of SD card:", clusterSize);
    if (hasUps && hasIps)
        log_debug(romPath, "Both .ups and .ips present; using .ups, ignoring .ips");
#endif

    if (hasUps)
    {
        u32 upsSize = sFnoUps.fsize;
        u32 upsTimestamp = sFnoUps.fdate | (sFnoUps.ftime << 16);

        if (!verifyGpo(sGpoPath, romSize, upsSize, upsTimestamp, (u8)GpoPatchKind::Ups, clusterSize))
        {
#ifndef NDEBUG
            log_debug(romPath, "GPO signature mismatch or file missing. Generating GPO patch from UPS...");
#endif
            bool crcMismatch = false;
            if (!createGpoFromUps(romPath, sUpsPath, sGpoPath, romSize, upsSize, upsTimestamp,
                                   clusterSize, &crcMismatch))
            {
#ifndef NDEBUG
                log_debug(romPath, crcMismatch ? "createGpoFromUps failed: CRC mismatch!" : "createGpoFromUps failed!");
#endif
                return crcMismatch ? GpoInitResult::FatalMismatch : GpoInitResult::Inactive;
            }
#ifndef NDEBUG
            log_debug(romPath, "createGpoFromUps succeeded!");
#endif
        }
#ifndef NDEBUG
        else
        {
            log_debug(romPath, "Existing GPO verified successfully.");
        }
#endif

        bool ok = activateGpo(sGpoPath, romSize);
#ifndef NDEBUG
        log_debug(romPath, ok ? "GPO Patch initialized successfully!" : "activateGpo failed!");
        if (ok) log_table(romPath, sMergedClusterTable);
#endif
        return ok ? GpoInitResult::Active : GpoInitResult::Inactive;
    }

    u32 ipsSize = sFnoIps.fsize;
    u32 ipsTimestamp = sFnoIps.fdate | (sFnoIps.ftime << 16);

    if (!verifyGpo(sGpoPath, romSize, ipsSize, ipsTimestamp, (u8)GpoPatchKind::Ips, clusterSize))
    {
#ifndef NDEBUG
        log_debug(romPath, "GPO signature mismatch or file missing. Generating GPO patch...");
#endif
        if (!createGpoFromIps(romPath, sIpsPath, sGpoPath, romSize, ipsSize, ipsTimestamp, clusterSize))
        {
#ifndef NDEBUG
            log_debug(romPath, "createGpo failed!");
#endif
            return GpoInitResult::Inactive;
        }
#ifndef NDEBUG
        log_debug(romPath, "createGpo succeeded!");
#endif
    }
#ifndef NDEBUG
    else
    {
        log_debug(romPath, "Existing GPO verified successfully.");
    }
#endif

    bool ok = activateGpo(sGpoPath, romSize);
#ifndef NDEBUG
    log_debug(romPath, ok ? "GPO Patch initialized successfully!" : "activateGpo failed!");
    if (ok) log_table(romPath, sMergedClusterTable);
#endif
    return ok ? GpoInitResult::Active : GpoInitResult::Inactive;
}

// After the linear chunk is loaded via f_read, FatFS serves cluster 0 from gFile.obj.sclust
// (bypassing the merged CLMT) because fptr==0 triggers the sclust fallback.  Any IPS patch
// that touches cluster 0 is therefore not applied.  This function corrects that by reading
// every patched cluster that falls inside the linear window directly from gGpoFile.
// Extension clusters inside the window (only possible on ROMs < 2MB) are also loaded here.
extern "C" [[gnu::section(".ewram")]] void gpo_patchLinearChunk(u32 clusterSize, u32 linearChunkSize)
{
    u32 romSize          = sActiveGpoHeader.romSize;
    u32 patchedRomSize   = sActiveGpoHeader.patchedRomSize > 0
                           ? sActiveGpoHeader.patchedRomSize : (u32)gFile.obj.objsize;
    u32 totalRomClusters     = (romSize       + clusterSize - 1) / clusterSize;
    u32 patchedTotalClusters = (patchedRomSize + clusterSize - 1) / clusterSize;

    // Precompute P_rom for extension cluster GPO offset calculation
    u32 patchedRomClusterCount = 0;
    for (u32 c = 0; c < totalRomClusters; c++)
        if (c < 8192 && (gGpoBitmask[c / 32] & (1u << (c % 32))))
            patchedRomClusterCount++;

    u32 maxLinearCluster = linearChunkSize / clusterSize;
    u32 limit = maxLinearCluster < patchedTotalClusters ? maxLinearCluster : patchedTotalClusters;
    if (limit > 8192u) limit = 8192u;  // bitmask has 8192 entries max

    u32 patchedIndex = 0;
    for (u32 c = 0; c < limit; c++)
    {
        if (c >= totalRomClusters)
        {
            // Extension cluster: sequential in GPO after all patched ROM clusters
            u32 gpoFileOffset = clusterSize * (1u + patchedRomClusterCount + (c - totalRomClusters));
            UINT br;
            if (f_lseek(&gGpoFile, gpoFileOffset) == FR_OK)
                f_read(&gGpoFile, (void*)(ROM_LINEAR_DS_ADDRESS + c * clusterSize), clusterSize, &br);
        }
        else if (c < 8192 && (gGpoBitmask[c / 32] & (1u << (c % 32))))
        {
            // Patched ROM cluster: sequential in GPO after the header cluster
            u32 gpoFileOffset = clusterSize * (1u + patchedIndex);
            UINT br;
            if (f_lseek(&gGpoFile, gpoFileOffset) == FR_OK)
                f_read(&gGpoFile, (void*)(ROM_LINEAR_DS_ADDRESS + c * clusterSize), clusterSize, &br);
            patchedIndex++;
        }
    }
}
