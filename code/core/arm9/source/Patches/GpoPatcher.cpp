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

    if (memcmp(sVerifyHeader.magic, GPO_MAGIC, 4) != 0)  { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.romSize      != romSize)            { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.ipsSize      != ipsSize)            { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.ipsTimestamp != ipsTime)            { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.clusterSize  != clusterSize)        { f_close(&sVerifyFile); return false; }

    // Compute P_rom: patched ROM cluster count, scoped to ROM region only
    u32 totalRomClusters = (romSize + clusterSize - 1) / clusterSize;
    u32 P_rom = 0;
    for (u32 c = 0; c < totalRomClusters; c++)
        if (c < 8192 && (sVerifyHeader.bitmask[c / 32] & (1u << (c % 32))))
            P_rom++;

    // Compute E: extension cluster count
    u32 pRS = sVerifyHeader.patchedRomSize > 0
        ? sVerifyHeader.patchedRomSize : romSize;
    u32 patchedTotalClusters = (pRS + clusterSize - 1) / clusterSize;
    u32 E = patchedTotalClusters > totalRomClusters
        ? patchedTotalClusters - totalRomClusters : 0;

    // GPO file must contain: 1 header cluster + P_rom patched clusters + E extension clusters
    u32 expectedSize = (1 + P_rom + E) * clusterSize;
    bool sizeOk = f_size(&sVerifyFile) >= expectedSize;
    f_close(&sVerifyFile);
    return sizeOk;
}

[[gnu::section(".ewram")]] static bool createGpo(const char* romPath, const char* ipsPath, const char* gpoPath, u32 romSize, u32 ipsSize, u32 ipsTime, u32 clusterSize)
{
    if (clusterSize < sizeof(GpoHeader)) return false;

    if (f_open(&sRomFile,      romPath, FA_READ  | FA_OPEN_EXISTING) != FR_OK) return false;
    if (f_open(&sIpsFile,      ipsPath, FA_READ  | FA_OPEN_EXISTING) != FR_OK) { f_close(&sRomFile); return false; }
    if (f_open(&sWriteGpoFile, gpoPath, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) { f_close(&sRomFile); f_close(&sIpsFile); return false; }

    auto cleanupAndAbort = [&]() -> bool {
        f_close(&sRomFile);
        f_close(&sIpsFile);
        f_close(&sWriteGpoFile);
        f_unlink(gpoPath);
        return false;
    };

    // Write placeholder magic; will be promoted to GPO_MAGIC only on final committed rewrite.
    // If power is lost mid-creation, verifyGpo rejects "GPOT" and triggers a clean rebuild next boot.
    memcpy(sCreateHeader.magic, GPO_MAGIC_PLACEHOLDER, 4);
    sCreateHeader.romSize = romSize;
    sCreateHeader.patchedRomSize = 0;  // filled in after Pass 1
    sCreateHeader.ipsSize = ipsSize;
    sCreateHeader.ipsTimestamp = ipsTime;
    sCreateHeader.clusterSize = clusterSize;
    memset(sCreateHeader.bitmask, 0, sizeof(sCreateHeader.bitmask));

    char ipsHeader[5];
    UINT br;
    if (f_read(&sIpsFile, ipsHeader, 5, &br) != FR_OK || br != 5 || memcmp(ipsHeader, "PATCH", 5) != 0)
        return cleanupAndAbort();

    u32 maxHunkExtent = 0;
    bool eofReached = false;

    // Pass 1: scan IPS to identify patched clusters and find max hunk extent
    while (true)
    {
        u8 offsetBuf[3];
        if (f_read(&sIpsFile, offsetBuf, 3, &br) != FR_OK || br != 3) break;
        u32 offset = ((u32)offsetBuf[0] << 16) | ((u32)offsetBuf[1] << 8) | offsetBuf[2];

        // SNESTool EOF ambiguity fix: treat 0x454F46 as EOF only if <= 3 bytes remain.
        // A real hunk at this offset would need at least 2 size bytes + 1 data byte.
        if (offset == 0x454F46 && (ipsSize - (u32)f_tell(&sIpsFile)) <= 3)
        {
            eofReached = true;
            break;
        }

        u8 sizeBuf[2];
        if (f_read(&sIpsFile, sizeBuf, 2, &br) != FR_OK || br != 2) break;
        u32 size = ((u32)sizeBuf[0] << 8) | sizeBuf[1];

        if (size > 0)
        {
            if (offset + size > maxHunkExtent) maxHunkExtent = offset + size;

            u32 startBlock = offset / 4096;
            u32 endBlock   = (offset + size - 1) / 4096;
            u32 blocksPerCluster = clusterSize / 4096;
            if (blocksPerCluster == 0) blocksPerCluster = 1;
            for (u32 b = startBlock; b <= endBlock; b++)
            {
                u32 c = b / blocksPerCluster;
                if (c < 8192)
                    sCreateHeader.bitmask[c / 32] |= (1u << (c % 32));
            }
            if (f_lseek(&sIpsFile, f_tell(&sIpsFile) + size) != FR_OK)
                return cleanupAndAbort();
        }
        else
        {
            u8 countBuf[2];
            if (f_read(&sIpsFile, countBuf, 2, &br) != FR_OK || br != 2) break;
            u32 count = ((u32)countBuf[0] << 8) | countBuf[1];
            if (count == 0) return cleanupAndAbort();  // corrupt RLE: infinite loop guard

            u8 val;
            if (f_read(&sIpsFile, &val, 1, &br) != FR_OK || br != 1) break;

            if (offset + count > maxHunkExtent) maxHunkExtent = offset + count;

            u32 startBlock = offset / 4096;
            u32 endBlock   = (offset + count - 1) / 4096;
            u32 blocksPerCluster = clusterSize / 4096;
            if (blocksPerCluster == 0) blocksPerCluster = 1;
            for (u32 b = startBlock; b <= endBlock; b++)
            {
                u32 c = b / blocksPerCluster;
                if (c < 8192)
                    sCreateHeader.bitmask[c / 32] |= (1u << (c % 32));
            }
        }
    }

    if (!eofReached) return cleanupAndAbort();

    // Truncate field: IPS may have 3 optional bytes after EOF marker specifying new ROM size
    u32 truncateValue = 0;
    {
        FSIZE_t remaining = ipsSize - f_tell(&sIpsFile);
        if (remaining == 3)
        {
            u8 truncBuf[3];
            if (f_read(&sIpsFile, truncBuf, 3, &br) == FR_OK && br == 3)
                truncateValue = ((u32)truncBuf[0] << 16) | ((u32)truncBuf[1] << 8) | truncBuf[2];
        }
    }

    // Effective ROM size after patch
    u32 patchedRomSize = truncateValue > 0
        ? truncateValue
        : (maxHunkExtent > romSize ? maxHunkExtent : romSize);
    sCreateHeader.patchedRomSize = patchedRomSize;

    u32 totalRomClusters     = (romSize        + clusterSize - 1) / clusterSize;
    u32 patchedTotalClusters = (patchedRomSize  + clusterSize - 1) / clusterSize;

    // Clear bitmask bits outside [0, min(totalRomClusters, patchedTotalClusters)).
    // Extension clusters do not use the bitmask; truncated-away clusters must not either.
    u32 cleanBound = totalRomClusters < patchedTotalClusters
        ? totalRomClusters : patchedTotalClusters;
    for (u32 c = cleanBound; c < 8192; c++)
        sCreateHeader.bitmask[c / 32] &= ~(1u << (c % 32));

    // Write placeholder header (GPOT magic); promoted to GPO1 only at the end
    UINT bw;
    if (f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw) != FR_OK
        || bw != sizeof(sCreateHeader))
        return cleanupAndAbort();

    // Pad header cluster to clusterSize
    memset(sTempBuf, 0, sizeof(sTempBuf));
    u32 paddingBytes = clusterSize - sizeof(sCreateHeader);
    while (paddingBytes > 0)
    {
        u32 chunk = paddingBytes > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : paddingBytes;
        if (f_write(&sWriteGpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
            return cleanupAndAbort();
        paddingBytes -= chunk;
    }

    // Range A: copy ROM clusters that the IPS touches, up to min(totalRomClusters, patchedTotalClusters)
    u32 rangeALimit = totalRomClusters < patchedTotalClusters
        ? totalRomClusters : patchedTotalClusters;
    for (u32 c = 0; c < rangeALimit; c++)
    {
        if (c >= 8192 || !(sCreateHeader.bitmask[c / 32] & (1u << (c % 32))))
            continue;

        if (f_lseek(&sRomFile, (FSIZE_t)c * clusterSize) != FR_OK)
            return cleanupAndAbort();

        u32 remaining = clusterSize;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
            memset(sTempBuf, 0xFF, chunk);  // pre-fill so short reads yield open-bus 0xFF
            if (f_read(&sRomFile, sTempBuf, chunk, &br) != FR_OK)
                return cleanupAndAbort();
            if (f_write(&sWriteGpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
                return cleanupAndAbort();
            remaining -= chunk;
        }
    }

    // Range B: extension clusters (past original ROM end), filled with 0xFF open-bus value.
    // The IPS apply phase (Pass 2) will overwrite the relevant bytes afterward.
    if (patchedTotalClusters > totalRomClusters)
    {
        memset(sTempBuf, 0xFF, sizeof(sTempBuf));
        for (u32 c = totalRomClusters; c < patchedTotalClusters; c++)
        {
            u32 remaining = clusterSize;
            while (remaining > 0)
            {
                u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
                if (f_write(&sWriteGpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
                    return cleanupAndAbort();
                remaining -= chunk;
            }
        }
    }

    // Pass 2: Apply IPS patches to GPO file
    if (f_lseek(&sIpsFile, 5) != FR_OK) return cleanupAndAbort();

    // Precompute patched ROM cluster count (scoped to ROM region only; extension clusters use no bitmask bits)
    u32 patchedRomClusterCount = 0;
    for (u32 c = 0; c < totalRomClusters; c++)
        if (c < 8192 && (sCreateHeader.bitmask[c / 32] & (1u << (c % 32))))
            patchedRomClusterCount++;

    // writePatchToGpo: apply a contiguous data buffer to the GPO file at the right cluster offset.
    // Returns false on any I/O error (caller must cleanupAndAbort).
    auto writePatchToGpo = [&](u32 romOffset, const u8* data, u32 dataSize) -> bool {
        u32 startCluster = romOffset / clusterSize;
        u32 endCluster   = (romOffset + dataSize - 1) / clusterSize;
        for (u32 c = startCluster; c <= endCluster; c++)
        {
            if (c >= patchedTotalClusters) continue;  // truncation guard

            u32 clusterStartOffset = c * clusterSize;
            u32 clusterEndOffset   = clusterStartOffset + clusterSize;
            u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
            u32 patchEnd   = (romOffset + dataSize) < clusterEndOffset
                             ? (romOffset + dataSize) : clusterEndOffset;
            if (patchStart >= patchEnd) continue;

            u32 gpoIdx;
            if (c >= totalRomClusters)
            {
                // Extension cluster: sequential after all patched ROM clusters in GPO
                gpoIdx = patchedRomClusterCount + (c - totalRomClusters);
            }
            else
            {
                // c < 8192 guard: small-cluster SD cards can have totalRomClusters > 8192
                if (c >= 8192 || !(sCreateHeader.bitmask[c / 32] & (1u << (c % 32)))) continue;
                gpoIdx = 0;
                for (u32 i = 0; i < c / 32; i++)
                    gpoIdx += gPopCountTable.PopCount(sCreateHeader.bitmask[i]);
                gpoIdx += gPopCountTable.PopCount(sCreateHeader.bitmask[c / 32] & ((1u << (c % 32)) - 1));
            }

            u32 targetFileOffset = clusterSize + gpoIdx * clusterSize + (patchStart - clusterStartOffset);
            UINT bw2;
            if (f_lseek(&sWriteGpoFile, targetFileOffset) != FR_OK) return false;
            if (f_write(&sWriteGpoFile, data + (patchStart - romOffset), patchEnd - patchStart, &bw2) != FR_OK
                || bw2 != (patchEnd - patchStart)) return false;
        }
        return true;
    };

    // writeRleToGpo: apply an RLE hunk to the GPO file.
    // Returns false on any I/O error.
    auto writeRleToGpo = [&](u32 romOffset, u8 val, u32 count) -> bool {
        u32 startCluster = romOffset / clusterSize;
        u32 endCluster   = (romOffset + count - 1) / clusterSize;
        for (u32 c = startCluster; c <= endCluster; c++)
        {
            if (c >= patchedTotalClusters) continue;  // truncation guard

            u32 clusterStartOffset = c * clusterSize;
            u32 clusterEndOffset   = clusterStartOffset + clusterSize;
            u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
            u32 patchEnd   = (romOffset + count) < clusterEndOffset
                             ? (romOffset + count) : clusterEndOffset;
            if (patchStart >= patchEnd) continue;

            u32 gpoIdx;
            if (c >= totalRomClusters)
            {
                gpoIdx = patchedRomClusterCount + (c - totalRomClusters);
            }
            else
            {
                if (c >= 8192 || !(sCreateHeader.bitmask[c / 32] & (1u << (c % 32)))) continue;
                gpoIdx = 0;
                for (u32 i = 0; i < c / 32; i++)
                    gpoIdx += gPopCountTable.PopCount(sCreateHeader.bitmask[i]);
                gpoIdx += gPopCountTable.PopCount(sCreateHeader.bitmask[c / 32] & ((1u << (c % 32)) - 1));
            }

            u32 targetFileOffset = clusterSize + gpoIdx * clusterSize + (patchStart - clusterStartOffset);
            UINT bw2;
            if (f_lseek(&sWriteGpoFile, targetFileOffset) != FR_OK) return false;

            u32 writeLen = patchEnd - patchStart;
            u32 fillSize = sizeof(sTempBuf) < clusterSize ? sizeof(sTempBuf) : clusterSize;
            memset(sTempBuf, val, fillSize);
            u32 remaining = writeLen;
            while (remaining > 0)
            {
                u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
                if (f_write(&sWriteGpoFile, sTempBuf, chunk, &bw2) != FR_OK || bw2 != chunk) return false;
                remaining -= chunk;
            }
        }
        return true;
    };

    bool pass2Eof = false;
    while (true)
    {
        u8 offsetBuf[3];
        if (f_read(&sIpsFile, offsetBuf, 3, &br) != FR_OK || br != 3) break;
        u32 offset = ((u32)offsetBuf[0] << 16) | ((u32)offsetBuf[1] << 8) | offsetBuf[2];

        if (offset == 0x454F46 && (ipsSize - (u32)f_tell(&sIpsFile)) <= 3)
        {
            pass2Eof = true;
            break;
        }

        u8 sizeBuf[2];
        if (f_read(&sIpsFile, sizeBuf, 2, &br) != FR_OK || br != 2) break;
        u32 size = ((u32)sizeBuf[0] << 8) | sizeBuf[1];

        if (size > 0)
        {
            u32 remaining = size;
            u32 currentOffset = offset;
            while (remaining > 0)
            {
                u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
                if (f_read(&sIpsFile, sTempBuf, chunk, &br) != FR_OK || br != chunk)
                    return cleanupAndAbort();
                if (!writePatchToGpo(currentOffset, sTempBuf, chunk))
                    return cleanupAndAbort();
                currentOffset += chunk;
                remaining -= chunk;
            }
        }
        else
        {
            u8 countBuf[2];
            if (f_read(&sIpsFile, countBuf, 2, &br) != FR_OK || br != 2) break;
            u32 count = ((u32)countBuf[0] << 8) | countBuf[1];
            if (count == 0) return cleanupAndAbort();

            u8 val;
            if (f_read(&sIpsFile, &val, 1, &br) != FR_OK || br != 1) break;

            if (!writeRleToGpo(offset, val, count))
                return cleanupAndAbort();
        }
    }

    // A truncated IPS (no EOF marker in Pass 2) must not commit the GPO
    if (!pass2Eof) return cleanupAndAbort();

    // Final header commit: promote placeholder magic to GPO_MAGIC
    memcpy(sCreateHeader.magic, GPO_MAGIC, 4);
    if (f_lseek(&sWriteGpoFile, 0) != FR_OK) return cleanupAndAbort();
    if (f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw) != FR_OK
        || bw != sizeof(sCreateHeader))
        return cleanupAndAbort();

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

    u32 activeRomSize = sVerifyHeader.patchedRomSize > 0
        ? sVerifyHeader.patchedRomSize : romSize;

    if (!mergeClusterMaps(romSize, activeRomSize, clusterSize))
    {
#ifndef NDEBUG
        log_debug(romPath, "mergeClusterMaps failed!");
#endif
        f_close(&gGpoFile);
        return false;
    }

    // Update SdCache bounds before swapping the CLMT; SdCache uses gFile.obj.objsize as
    // the upper bound for read validation. Must be set before gFile.cltbl so that the
    // first f_read after the swap sees the correct limit.
    gFile.obj.objsize = activeRomSize;
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
// every patched cluster that falls inside the linear window directly from gGpoFile.
// Extension clusters inside the window (only possible on ROMs < 2MB) are also loaded here.
extern "C" [[gnu::section(".ewram")]] void gpo_patchLinearChunk(u32 clusterSize, u32 linearChunkSize)
{
    u32 romSize          = sVerifyHeader.romSize;
    u32 patchedRomSize   = sVerifyHeader.patchedRomSize > 0
                           ? sVerifyHeader.patchedRomSize : (u32)gFile.obj.objsize;
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
