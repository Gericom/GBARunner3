#include "GpoPatcher.h"
#include "PopCountTable.h"
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

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static u8 sTempBuf[4096];

// Static allocations in EWRAM BSS to prevent stack overflow in DTCM
[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sVerifyFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sRomFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sIpsFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sWriteGpoFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sLogFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FILINFO sFnoIps;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FILINFO sFnoRom;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sIpsPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sGpoPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sLogPath[256];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sVerifyHeader;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sCreateHeader;

extern FIL gFile; // Holds the GBA ROM file object and its cltbl

[[gnu::section(".ewram")]] static void gpo_getPath(const char* romPath, char* outPath, const char* ext)
{
    strcpy(outPath, romPath);
    char* dot = strrchr(outPath, '.');
    if (dot)
    {
        strcpy(dot, ext);
    }
    else
    {
        strcat(outPath, ext);
    }
}

[[gnu::section(".ewram")]] static void u32_to_str(u32 val, char* buf)
{
    char temp[16];
    int i = 0;
    if (val == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (val > 0)
    {
        temp[i++] = '0' + (val % 10);
        val /= 10;
    }
    for (int j = 0; j < i; j++)
    {
        buf[j] = temp[i - 1 - j];
    }
    buf[i] = '\0';
}

[[gnu::section(".ewram")]] static void log_debug(const char* romPath, const char* msg, u32 val1 = 0xFFFFFFFF, u32 val2 = 0xFFFFFFFF)
{
    gpo_getPath(romPath, sLogPath, "_gpo.log");
    if (f_open(&sLogFile, sLogPath, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
    {
        f_lseek(&sLogFile, f_size(&sLogFile));
        UINT bw;
        f_write(&sLogFile, msg, strlen(msg), &bw);
        if (val1 != 0xFFFFFFFF)
        {
            char valBuf[16];
            u32_to_str(val1, valBuf);
            f_write(&sLogFile, " val1: ", 7, &bw);
            f_write(&sLogFile, valBuf, strlen(valBuf), &bw);
        }
        if (val2 != 0xFFFFFFFF)
        {
            char valBuf[16];
            u32_to_str(val2, valBuf);
            f_write(&sLogFile, " val2: ", 7, &bw);
            f_write(&sLogFile, valBuf, strlen(valBuf), &bw);
        }
        f_write(&sLogFile, "\n", 1, &bw);
        f_close(&sLogFile);
    }
}

[[gnu::section(".ewram")]] static void log_table(const char* romPath, const DWORD* tbl)
{
    gpo_getPath(romPath, sLogPath, "_gpo.log");
    if (f_open(&sLogFile, sLogPath, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
    {
        f_lseek(&sLogFile, f_size(&sLogFile));
        UINT bw;
        f_write(&sLogFile, "--- Merged Linkmap ---\n", 23, &bw);
        u32 totalDwords = tbl[0];
        char szBuf[16];
        u32_to_str(totalDwords, szBuf);
        f_write(&sLogFile, "Total DWORDs: ", 14, &bw);
        f_write(&sLogFile, szBuf, strlen(szBuf), &bw);
        f_write(&sLogFile, "\n", 1, &bw);
        
        for (u32 i = 1; i < totalDwords; i += 2)
        {
            char idxBuf[16], sizeBuf[16], startBuf[16];
            u32_to_str(i / 2, idxBuf);
            u32_to_str(tbl[i], sizeBuf);
            u32_to_str(tbl[i+1], startBuf);
            
            f_write(&sLogFile, " Frag ", 6, &bw);
            f_write(&sLogFile, idxBuf, strlen(idxBuf), &bw);
            f_write(&sLogFile, ": size=", 7, &bw);
            f_write(&sLogFile, sizeBuf, strlen(sizeBuf), &bw);
            f_write(&sLogFile, " startCl=", 9, &bw);
            f_write(&sLogFile, startBuf, strlen(startBuf), &bw);
            f_write(&sLogFile, "\n", 1, &bw);
        }
        f_write(&sLogFile, "----------------------\n", 23, &bw);
        f_close(&sLogFile);
    }
}

[[gnu::section(".ewram")]] static bool verifyGpo(const char* gpoPath, u32 romSize, u32 ipsSize, u32 ipsTime, u32 clusterSize)
{
    if (f_open(&sVerifyFile, gpoPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return false;

    UINT br;
    if (f_read(&sVerifyFile, &sVerifyHeader, sizeof(sVerifyHeader), &br) != FR_OK || br != sizeof(sVerifyHeader))
    {
        f_close(&sVerifyFile);
        return false;
    }
    f_close(&sVerifyFile);

    if (memcmp(sVerifyHeader.magic, GPO_MAGIC, 4) != 0)
        return false;
    if (sVerifyHeader.romSize != romSize)
        return false;
    if (sVerifyHeader.ipsSize != ipsSize)
        return false;
    if (sVerifyHeader.ipsTimestamp != ipsTime)
        return false;
    if (sVerifyHeader.clusterSize != clusterSize)
        return false;

    return true;
}

[[gnu::section(".ewram")]] static bool createGpo(const char* romPath, const char* ipsPath, const char* gpoPath, u32 romSize, u32 ipsSize, u32 ipsTime, u32 clusterSize)
{
    if (f_open(&sRomFile, romPath, FA_READ | FA_OPEN_EXISTING) != FR_OK) return false;
    if (f_open(&sIpsFile, ipsPath, FA_READ | FA_OPEN_EXISTING) != FR_OK) { f_close(&sRomFile); return false; }
    if (f_open(&sWriteGpoFile, gpoPath, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) { f_close(&sRomFile); f_close(&sIpsFile); return false; }

    memcpy(sCreateHeader.magic, GPO_MAGIC, 4);
    sCreateHeader.romSize = romSize;
    sCreateHeader.ipsSize = ipsSize;
    sCreateHeader.ipsTimestamp = ipsTime;
    sCreateHeader.clusterSize = clusterSize;
    memset(sCreateHeader.bitmask, 0, sizeof(sCreateHeader.bitmask));

    char ipsHeader[5];
    UINT br;
    if (f_read(&sIpsFile, ipsHeader, 5, &br) != FR_OK || br != 5 || memcmp(ipsHeader, "PATCH", 5) != 0)
    {
        f_close(&sRomFile); f_close(&sIpsFile); f_close(&sWriteGpoFile);
        return false;
    }

    u32 blocksPerCluster = clusterSize / 4096;
    if (blocksPerCluster == 0) blocksPerCluster = 1;

    // Scan IPS to identify patched clusters
    while (true)
    {
        u8 offsetBuf[3];
        if (f_read(&sIpsFile, offsetBuf, 3, &br) != FR_OK || br != 3) break;
        u32 offset = (offsetBuf[0] << 16) | (offsetBuf[1] << 8) | offsetBuf[2];
        if (offset == 0x454F46)
            break;

        u8 sizeBuf[2];
        if (f_read(&sIpsFile, sizeBuf, 2, &br) != FR_OK || br != 2) break;
        u32 size = (sizeBuf[0] << 8) | sizeBuf[1];

        if (size > 0)
        {
            u32 startBlock = offset / 4096;
            u32 endBlock = (offset + size - 1) / 4096;
            for (u32 b = startBlock; b <= endBlock; b++)
            {
                u32 c = b / blocksPerCluster;
                if (c < 8192)
                    sCreateHeader.bitmask[c / 32] |= (1u << (c % 32));
            }
            if (f_lseek(&sIpsFile, f_tell(&sIpsFile) + size) != FR_OK)
            {
                f_close(&sRomFile); f_close(&sIpsFile); f_close(&sWriteGpoFile);
                return false;
            }
        }
        else
        {
            u8 countBuf[2];
            if (f_read(&sIpsFile, countBuf, 2, &br) != FR_OK || br != 2) break;
            u32 count = (countBuf[0] << 8) | countBuf[1];
            
            u8 val;
            if (f_read(&sIpsFile, &val, 1, &br) != FR_OK || br != 1) break;

            u32 startBlock = offset / 4096;
            u32 endBlock = (offset + count - 1) / 4096;
            for (u32 b = startBlock; b <= endBlock; b++)
            {
                u32 c = b / blocksPerCluster;
                if (c < 8192)
                    sCreateHeader.bitmask[c / 32] |= (1u << (c % 32));
            }
        }
    }

    // Write header
    UINT bw;
    f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw);

    // Pad header to clusterSize using sTempBuf (already EWRAM static)
    memset(sTempBuf, 0, sizeof(sTempBuf));
    u32 paddingBytes = clusterSize - sizeof(sCreateHeader);
    while (paddingBytes > 0)
    {
        u32 chunk = paddingBytes > sizeof(sTempBuf) ? sizeof(sTempBuf) : paddingBytes;
        f_write(&sWriteGpoFile, sTempBuf, chunk, &bw);
        paddingBytes -= chunk;
    }

    // Write original GBA ROM clusters for patched clusters
    u32 totalClusters = (romSize + clusterSize - 1) / clusterSize;
    for (u32 c = 0; c < totalClusters; c++)
    {
        if (sCreateHeader.bitmask[c / 32] & (1u << (c % 32)))
        {
            f_lseek(&sRomFile, c * clusterSize);
            u32 remaining = clusterSize;
            while (remaining > 0)
            {
                u32 chunk = remaining > 4096 ? 4096 : remaining;
                memset(sTempBuf, 0xFF, chunk);
                f_read(&sRomFile, sTempBuf, chunk, &br);
                f_write(&sWriteGpoFile, sTempBuf, chunk, &bw);
                remaining -= chunk;
            }
        }
    }

    // Apply patches to GPO
    f_lseek(&sIpsFile, 5);

    auto writePatchToGpo = [&](u32 romOffset, const u8* data, u32 dataSize) {
        u32 startCluster = romOffset / clusterSize;
        u32 endCluster = (romOffset + dataSize - 1) / clusterSize;
        for (u32 c = startCluster; c <= endCluster; c++)
        {
            if (c >= 8192) continue;
            if (!(sCreateHeader.bitmask[c / 32] & (1u << (c % 32)))) continue;

            u32 patchedClusterIndex = 0;
            for (u32 i = 0; i < c / 32; i++)
                patchedClusterIndex += gPopCountTable.PopCount(sCreateHeader.bitmask[i]);
            patchedClusterIndex += gPopCountTable.PopCount(sCreateHeader.bitmask[c / 32] & ((1u << (c % 32)) - 1));
            
            u32 clusterStartOffset = c * clusterSize;
            u32 clusterEndOffset = clusterStartOffset + clusterSize;
            
            u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
            u32 patchEnd = (romOffset + dataSize) < clusterEndOffset ? (romOffset + dataSize) : clusterEndOffset;
            
            if (patchStart < patchEnd)
            {
                u32 targetFileOffset = clusterSize + patchedClusterIndex * clusterSize + (patchStart - clusterStartOffset);
                f_lseek(&sWriteGpoFile, targetFileOffset);
                f_write(&sWriteGpoFile, data + (patchStart - romOffset), patchEnd - patchStart, &bw);
            }
        }
    };

    auto writeRleToGpo = [&](u32 romOffset, u8 val, u32 count) {
        u32 startCluster = romOffset / clusterSize;
        u32 endCluster = (romOffset + count - 1) / clusterSize;
        for (u32 c = startCluster; c <= endCluster; c++)
        {
            if (c >= 8192) continue;
            if (!(sCreateHeader.bitmask[c / 32] & (1u << (c % 32)))) continue;

            u32 patchedClusterIndex = 0;
            for (u32 i = 0; i < c / 32; i++)
                patchedClusterIndex += gPopCountTable.PopCount(sCreateHeader.bitmask[i]);
            patchedClusterIndex += gPopCountTable.PopCount(sCreateHeader.bitmask[c / 32] & ((1u << (c % 32)) - 1));
            
            u32 clusterStartOffset = c * clusterSize;
            u32 clusterEndOffset = clusterStartOffset + clusterSize;
            
            u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
            u32 patchEnd = (romOffset + count) < clusterEndOffset ? (romOffset + count) : clusterEndOffset;
            
            if (patchStart < patchEnd)
            {
                u32 targetFileOffset = clusterSize + patchedClusterIndex * clusterSize + (patchStart - clusterStartOffset);
                f_lseek(&sWriteGpoFile, targetFileOffset);
                u32 writeLen = patchEnd - patchStart;
                
                u32 remaining = writeLen;
                memset(sTempBuf, val, clusterSize < 4096 ? clusterSize : 4096);
                while (remaining > 0)
                {
                    u32 chunk = remaining > 4096 ? 4096 : remaining;
                    f_write(&sWriteGpoFile, sTempBuf, chunk, &bw);
                    remaining -= chunk;
                }
            }
        }
    };

    while (true)
    {
        u8 offsetBuf[3];
        if (f_read(&sIpsFile, offsetBuf, 3, &br) != FR_OK || br != 3) break;
        u32 offset = (offsetBuf[0] << 16) | (offsetBuf[1] << 8) | offsetBuf[2];
        if (offset == 0x454F46)
            break;

        u8 sizeBuf[2];
        if (f_read(&sIpsFile, sizeBuf, 2, &br) != FR_OK || br != 2) break;
        u32 size = (sizeBuf[0] << 8) | sizeBuf[1];

        if (size > 0)
        {
            u32 remaining = size;
            u32 currentOffset = offset;
            while (remaining > 0)
            {
                u32 chunk = remaining > 4096 ? 4096 : remaining;
                f_read(&sIpsFile, sTempBuf, chunk, &br);
                writePatchToGpo(currentOffset, sTempBuf, chunk);
                currentOffset += chunk;
                remaining -= chunk;
            }
        }
        else
        {
            u8 countBuf[2];
            if (f_read(&sIpsFile, countBuf, 2, &br) != FR_OK || br != 2) break;
            u32 count = (countBuf[0] << 8) | countBuf[1];
            
            u8 val;
            f_read(&sIpsFile, &val, 1, &br);
            
            writeRleToGpo(offset, val, count);
        }
    }

    f_lseek(&sWriteGpoFile, 0);
    f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw);

    f_close(&sRomFile);
    f_close(&sIpsFile);
    f_close(&sWriteGpoFile);
    return true;
}

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

[[gnu::section(".ewram")]] static bool mergeClusterMaps(u32 romSize, u32 clusterSize)
{
    u32 totalClusters = (romSize + clusterSize - 1) / clusterSize;
    
    DWORD* dest = sMergedClusterTable + 1;
    u32 tableLimit = 2048 - 2;
    
    u32 fragmentCount = 0;
    u32 currentFragSize = 0;
    DWORD currentFragStartPhys = 0;
    
    u32 patchedClusterIndex = 0;
    
    ClmtTracker romTracker;
    romTracker.Init(gFile.cltbl);
    
    ClmtTracker gpoTracker;
    gpoTracker.Init(sGpoClusterTable);
    
    for (u32 c = 0; c < totalClusters; c++)
    {
        bool isPatched = c < 8192 && (gGpoBitmask[c / 32] & (1u << (c % 32)));
        
        DWORD physCl = 0;
        if (isPatched)
        {
            physCl = gpoTracker.GetPhysicalCluster(1 + patchedClusterIndex);
            patchedClusterIndex++;
        }
        else
        {
            physCl = romTracker.GetPhysicalCluster(c);
        }
        
        if (physCl == 0)
        {
            return false;
        }
        
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
            if ((u32)(dest - sMergedClusterTable) >= tableLimit)
            {
                return false;
            }
            *dest++ = currentFragSize;
            *dest++ = currentFragStartPhys;
            fragmentCount++;
            
            currentFragSize = 1;
            currentFragStartPhys = physCl;
        }
    }
    
    if (currentFragSize > 0)
    {
        if ((u32)(dest - sMergedClusterTable) >= tableLimit)
        {
            return false;
        }
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

    // Backup original cltbl
    DWORD* origCltbl = gFile.cltbl;

    // Mock GBA ROM cluster table (representing 1000 clusters contiguously starting at cluster 100)
    DWORD gbaCltblMock[4];
    gbaCltblMock[0] = 3;
    gbaCltblMock[1] = 1000;
    gbaCltblMock[2] = 100;
    gbaCltblMock[3] = 0;
    gFile.cltbl = gbaCltblMock;

    // Mock GPO cluster table (representing 2000 clusters contiguously starting at cluster 5000)
    sGpoClusterTable[0] = 3;
    sGpoClusterTable[1] = 2000;
    sGpoClusterTable[2] = 5000;
    sGpoClusterTable[3] = 0;

    // Test 1: Merge under limit (200 clusters, alternating)
    // Alternate every odd cluster
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 200; i++)
    {
        if (i % 2 == 1)
        {
            gGpoBitmask[i / 32] |= (1u << (i % 32));
        }
    }
    bool res1 = mergeClusterMaps(200 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] Merge under limit (200 clusters, alternating): %s (expected: true)\n", res1 ? "PASS" : "FAIL");

    // Test 2: Merge over limit to trigger overflow
    // Alternating 1100 clusters produces ~1100 fragments; limit is 1023, so this must overflow.
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
    for (u32 i = 0; i < 1100; i++)
    {
        if (i % 2 == 1)
        {
            gGpoBitmask[i / 32] |= (1u << (i % 32));
        }
    }
    bool res2 = mergeClusterMaps(1100 * 4096, 4096);
    gLogger->Log(LogLevel::Debug, "[GPO Test] Merge over limit (1100 clusters, alternating): %s (expected: false)\n", !res2 ? "PASS" : "FAIL");

    // Restore original state
    gFile.cltbl = origCltbl;
    memset(gGpoBitmask, 0, sizeof(gGpoBitmask));
}
#endif

extern "C" [[gnu::section(".ewram")]] bool gpo_init(const char* romPath)
{
    gpo_getPath(romPath, sIpsPath, ".ips");

    if (f_stat(sIpsPath, &sFnoIps) != FR_OK)
    {
        return false;
    }

#ifndef NDEBUG
    gpo_runUnitTests();
#endif

    gpo_getPath(romPath, sGpoPath, ".gpo");

#ifndef NDEBUG
    gpo_getPath(romPath, sLogPath, "_gpo.log");
    f_unlink(sLogPath);
    log_debug(romPath, "=== GPO INIT ===");
#endif

    if (f_stat(romPath, &sFnoRom) != FR_OK)
    {
#ifndef NDEBUG
        log_debug(romPath, "No ROM file found at path:", 0, 0);
#endif
        return false;
    }

    u32 romSize = sFnoRom.fsize;
    u32 ipsSize = sFnoIps.fsize;
    u32 ipsTime = sFnoIps.fdate | (sFnoIps.ftime << 16);
    u32 clusterSize = gFile.obj.fs->csize * 512;

#ifndef NDEBUG
    log_debug(romPath, "ROM/IPS size loaded:", romSize, ipsSize);
    log_debug(romPath, "Cluster size of SD card:", clusterSize);
#endif

    if (!verifyGpo(sGpoPath, romSize, ipsSize, ipsTime, clusterSize))
    {
#ifndef NDEBUG
        log_debug(romPath, "GPO signature mismatch or file missing. Generating GPO patch...");
#endif
        if (!createGpo(romPath, sIpsPath, sGpoPath, romSize, ipsSize, ipsTime, clusterSize))
        {
#ifndef NDEBUG
            log_debug(romPath, "createGpo failed!");
#endif
            return false;
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

    if (f_open(&gGpoFile, sGpoPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
    {
#ifndef NDEBUG
        log_debug(romPath, "f_open gGpoFile failed!");
#endif
        return false;
    }

    UINT br;
    if (f_read(&gGpoFile, &sVerifyHeader, sizeof(sVerifyHeader), &br) != FR_OK || br != sizeof(sVerifyHeader))
    {
#ifndef NDEBUG
        log_debug(romPath, "f_read GpoHeader failed!");
#endif
        f_close(&gGpoFile);
        return false;
    }

    memcpy(gGpoBitmask, sVerifyHeader.bitmask, sizeof(gGpoBitmask));

    sGpoClusterTable[0] = sizeof(sGpoClusterTable) / sizeof(DWORD);
    gGpoFile.cltbl = sGpoClusterTable;
    if (f_lseek(&gGpoFile, CREATE_LINKMAP) != FR_OK)
    {
#ifndef NDEBUG
        log_debug(romPath, "f_lseek CREATE_LINKMAP failed on gGpoFile!");
#endif
        f_close(&gGpoFile);
        return false;
    }

#ifndef NDEBUG
    log_debug(romPath, "sGpoClusterTable[0] size after CREATE_LINKMAP:", sGpoClusterTable[0]);
#endif

    if (!mergeClusterMaps(romSize, clusterSize))
    {
#ifndef NDEBUG
        log_debug(romPath, "mergeClusterMaps failed!");
#endif
        f_close(&gGpoFile);
        return false;
    }

    gFile.cltbl = sMergedClusterTable;

#ifndef NDEBUG
    log_debug(romPath, "mergeClusterMaps succeeded!");
    log_table(romPath, sMergedClusterTable);
    log_debug(romPath, "GPO Patch initialized successfully!");
#endif

    return true;
}

// After the linear chunk is loaded via f_read, FatFS serves cluster 0 from gFile.obj.sclust
// (bypassing the merged CLMT) because fptr==0 triggers the sclust fallback.  Any IPS patch
// that touches cluster 0 is therefore not applied.  This function corrects that by reading
// every patched cluster that falls inside the linear window directly from gGpoFile, whose
// own CLMT (sGpoClusterTable) has no fptr==0 ambiguity for these seeks.
extern "C" [[gnu::section(".ewram")]] void gpo_patchLinearChunk(u32 clusterSize, u32 linearChunkSize)
{
    u32 maxLinearCluster = linearChunkSize / clusterSize;
    u32 limit = maxLinearCluster < 8192u ? maxLinearCluster : 8192u;
    u32 patchedIndex = 0;

    for (u32 c = 0; c < limit; c++)
    {
        if (gGpoBitmask[c / 32] & (1u << (c % 32)))
        {
            // GPO file layout: one header cluster (padded), then patched ROM clusters in order.
            // patchedIndex 0 -> GPO file offset clusterSize (first data cluster).
            u32 gpoFileOffset = clusterSize * (1u + patchedIndex);
            UINT br;
            if (f_lseek(&gGpoFile, gpoFileOffset) == FR_OK)
            {
                f_read(&gGpoFile, (void*)(ROM_LINEAR_DS_ADDRESS + c * clusterSize), clusterSize, &br);
            }
            patchedIndex++;
        }
    }
}
