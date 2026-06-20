#include "GpoBuilder.h"
#include "PopCountTable.h"
#include <string.h>

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sLogFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static char sLogPath[256];

[[gnu::section(".ewram")]] void gpo_getPath(const char* romPath, char* outPath, const char* ext)
{
    strcpy(outPath, romPath);
    char* dot = strrchr(outPath, '.');
    if (dot)
        strcpy(dot, ext);
    else
        strcat(outPath, ext);
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
        buf[j] = temp[i - 1 - j];
    buf[i] = '\0';
}

[[gnu::section(".ewram")]] void log_debug(const char* romPath, const char* msg, u32 val1, u32 val2)
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

[[gnu::section(".ewram")]] void log_table(const char* romPath, const DWORD* tbl)
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

[[gnu::section(".ewram")]] bool gpoClusterFileOffset(const GpoHeader& header, u32 cluster, u32* outOffset)
{
    u32 totalRomClusters = (header.romSize + header.clusterSize - 1) / header.clusterSize;
    u32 patchedRomSize = header.patchedRomSize > 0 ? header.patchedRomSize : header.romSize;
    u32 patchedTotalClusters = (patchedRomSize + header.clusterSize - 1) / header.clusterSize;

    if (cluster >= patchedTotalClusters) return false;

    if (cluster >= totalRomClusters)
    {
        u32 patchedRomClusterCount = 0;
        for (u32 c = 0; c < totalRomClusters; c++)
            if (c < 8192 && (header.bitmask[c / 32] & (1u << (c % 32))))
                patchedRomClusterCount++;
        *outOffset = header.clusterSize * (1u + patchedRomClusterCount + (cluster - totalRomClusters));
        return true;
    }

    if (cluster >= 8192 || !(header.bitmask[cluster / 32] & (1u << (cluster % 32))))
        return false;

    u32 gpoIdx = 0;
    for (u32 i = 0; i < cluster / 32; i++)
        gpoIdx += gPopCountTable.PopCount(header.bitmask[i]);
    gpoIdx += gPopCountTable.PopCount(header.bitmask[cluster / 32] & ((1u << (cluster % 32)) - 1));

    *outOffset = header.clusterSize * (1u + gpoIdx);
    return true;
}

[[gnu::section(".ewram")]] bool writePatchToGpo(FIL* gpoFile, const GpoHeader& header, u32 romOffset,
                                                  const u8* data, u32 dataSize)
{
    u32 startCluster = romOffset / header.clusterSize;
    u32 endCluster   = (romOffset + dataSize - 1) / header.clusterSize;
    for (u32 c = startCluster; c <= endCluster; c++)
    {
        u32 clusterStartOffset = c * header.clusterSize;
        u32 clusterEndOffset   = clusterStartOffset + header.clusterSize;
        u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
        u32 patchEnd   = (romOffset + dataSize) < clusterEndOffset ? (romOffset + dataSize) : clusterEndOffset;
        if (patchStart >= patchEnd) continue;

        u32 gpoClusterOffset;
        if (!gpoClusterFileOffset(header, c, &gpoClusterOffset)) continue;

        u32 targetFileOffset = gpoClusterOffset + (patchStart - clusterStartOffset);
        UINT bw;
        if (f_lseek(gpoFile, targetFileOffset) != FR_OK) return false;
        if (f_write(gpoFile, data + (patchStart - romOffset), patchEnd - patchStart, &bw) != FR_OK
            || bw != (patchEnd - patchStart)) return false;
    }
    return true;
}

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static u8 sTempBuf[4096];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sVerifyFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sRomFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sIpsFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sWriteGpoFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sVerifyHeader;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sCreateHeader;

[[gnu::section(".ewram")]] bool verifyGpo(const char* gpoPath, u32 romSize, u32 patchSize,
                                            u32 patchTimestamp, u8 patchKind, u32 clusterSize)
{
    if (f_open(&sVerifyFile, gpoPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return false;

    UINT br;
    if (f_read(&sVerifyFile, &sVerifyHeader, sizeof(sVerifyHeader), &br) != FR_OK || br != sizeof(sVerifyHeader))
    {
        f_close(&sVerifyFile);
        return false;
    }

    if (memcmp(sVerifyHeader.magic, GPO_MAGIC, 4) != 0)        { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.romSize        != romSize)               { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.patchSize      != patchSize)              { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.patchTimestamp != patchTimestamp)         { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.patchKind      != patchKind)              { f_close(&sVerifyFile); return false; }
    if (sVerifyHeader.clusterSize    != clusterSize)            { f_close(&sVerifyFile); return false; }

    u32 totalRomClusters = (romSize + clusterSize - 1) / clusterSize;
    u32 P_rom = 0;
    for (u32 c = 0; c < totalRomClusters; c++)
        if (c < 8192 && (sVerifyHeader.bitmask[c / 32] & (1u << (c % 32))))
            P_rom++;

    u32 pRS = sVerifyHeader.patchedRomSize > 0 ? sVerifyHeader.patchedRomSize : romSize;
    u32 patchedTotalClusters = (pRS + clusterSize - 1) / clusterSize;
    u32 E = patchedTotalClusters > totalRomClusters ? patchedTotalClusters - totalRomClusters : 0;

    u32 expectedSize = (1 + P_rom + E) * clusterSize;
    bool sizeOk = f_size(&sVerifyFile) >= expectedSize;
    f_close(&sVerifyFile);
    return sizeOk;
}

[[gnu::section(".ewram")]] bool copyRangeAClusters(FIL* gpoFile, FIL* romFile, const GpoHeader& header)
{
    u32 totalRomClusters = (header.romSize + header.clusterSize - 1) / header.clusterSize;
    u32 patchedRomSize = header.patchedRomSize > 0 ? header.patchedRomSize : header.romSize;
    u32 patchedTotalClusters = (patchedRomSize + header.clusterSize - 1) / header.clusterSize;
    u32 rangeALimit = totalRomClusters < patchedTotalClusters ? totalRomClusters : patchedTotalClusters;

    for (u32 c = 0; c < rangeALimit; c++)
    {
        if (c >= 8192 || !(header.bitmask[c / 32] & (1u << (c % 32))))
            continue;

        if (f_lseek(romFile, (FSIZE_t)c * header.clusterSize) != FR_OK)
            return false;

        u32 remaining = header.clusterSize;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
            UINT br, bw;
            memset(sTempBuf, 0xFF, chunk);
            if (f_read(romFile, sTempBuf, chunk, &br) != FR_OK)
                return false;
            if (f_write(gpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
                return false;
            remaining -= chunk;
        }
    }
    return true;
}

[[gnu::section(".ewram")]] bool writeRangeBExtensionClusters(FIL* gpoFile, const GpoHeader& header)
{
    u32 totalRomClusters = (header.romSize + header.clusterSize - 1) / header.clusterSize;
    u32 patchedRomSize = header.patchedRomSize > 0 ? header.patchedRomSize : header.romSize;
    u32 patchedTotalClusters = (patchedRomSize + header.clusterSize - 1) / header.clusterSize;

    if (patchedTotalClusters <= totalRomClusters) return true;

    memset(sTempBuf, 0xFF, sizeof(sTempBuf));
    for (u32 c = totalRomClusters; c < patchedTotalClusters; c++)
    {
        u32 remaining = header.clusterSize;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
            UINT bw;
            if (f_write(gpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
                return false;
            remaining -= chunk;
        }
    }
    return true;
}

[[gnu::section(".ewram")]] static bool writeRleToGpo(FIL* gpoFile, const GpoHeader& header, u32 romOffset,
                                                        u8 val, u32 count)
{
    u32 startCluster = romOffset / header.clusterSize;
    u32 endCluster   = (romOffset + count - 1) / header.clusterSize;
    for (u32 c = startCluster; c <= endCluster; c++)
    {
        u32 clusterStartOffset = c * header.clusterSize;
        u32 clusterEndOffset   = clusterStartOffset + header.clusterSize;
        u32 patchStart = romOffset > clusterStartOffset ? romOffset : clusterStartOffset;
        u32 patchEnd   = (romOffset + count) < clusterEndOffset ? (romOffset + count) : clusterEndOffset;
        if (patchStart >= patchEnd) continue;

        u32 gpoClusterOffset;
        if (!gpoClusterFileOffset(header, c, &gpoClusterOffset)) continue;

        u32 targetFileOffset = gpoClusterOffset + (patchStart - clusterStartOffset);
        UINT bw;
        if (f_lseek(gpoFile, targetFileOffset) != FR_OK) return false;

        u32 writeLen = patchEnd - patchStart;
        u32 fillSize = sizeof(sTempBuf) < header.clusterSize ? sizeof(sTempBuf) : header.clusterSize;
        memset(sTempBuf, val, fillSize);
        u32 remaining = writeLen;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : remaining;
            if (f_write(gpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk) return false;
            remaining -= chunk;
        }
    }
    return true;
}

[[gnu::section(".ewram")]] bool createGpoFromIps(const char* romPath, const char* ipsPath, const char* gpoPath,
                                                    u32 romSize, u32 ipsSize, u32 ipsTime, u32 clusterSize)
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
    sCreateHeader.patchSize = ipsSize;
    sCreateHeader.patchTimestamp = ipsTime;
    sCreateHeader.patchKind = (u8)GpoPatchKind::Ips;
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
    u32 patchedTotalClusters = (patchedRomSize + clusterSize - 1) / clusterSize;

    // Clear bitmask bits outside [0, min(totalRomClusters, patchedTotalClusters)).
    // Extension clusters do not use the bitmask; truncated-away clusters must not either.
    u32 cleanBound = totalRomClusters < patchedTotalClusters
        ? totalRomClusters : patchedTotalClusters;
    for (u32 c = cleanBound; c < 8192; c++)
        sCreateHeader.bitmask[c / 32] &= ~(1u << (c % 32));

    // Write placeholder header (GPOT magic); promoted to GPO2 only at the end
    UINT bw;
    if (f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw) != FR_OK
        || bw != sizeof(sCreateHeader))
        return cleanupAndAbort();

    memset(sTempBuf, 0, sizeof(sTempBuf));
    u32 paddingBytes = clusterSize - sizeof(sCreateHeader);
    while (paddingBytes > 0)
    {
        u32 chunk = paddingBytes > sizeof(sTempBuf) ? (u32)sizeof(sTempBuf) : paddingBytes;
        if (f_write(&sWriteGpoFile, sTempBuf, chunk, &bw) != FR_OK || bw != chunk)
            return cleanupAndAbort();
        paddingBytes -= chunk;
    }

    if (!copyRangeAClusters(&sWriteGpoFile, &sRomFile, sCreateHeader)) return cleanupAndAbort();
    if (!writeRangeBExtensionClusters(&sWriteGpoFile, sCreateHeader)) return cleanupAndAbort();

    // Pass 2: Apply IPS patches to GPO file
    if (f_lseek(&sIpsFile, 5) != FR_OK) return cleanupAndAbort();

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
                if (!writePatchToGpo(&sWriteGpoFile, sCreateHeader, currentOffset, sTempBuf, chunk))
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

            if (!writeRleToGpo(&sWriteGpoFile, sCreateHeader, offset, val, count))
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
