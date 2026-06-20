#include "UpsPatcher.h"
#include "GpoBuilder.h"
#include "Crc32Table.h"
#include <string.h>

// Start with crc = 0xFFFFFFFF; after all data is fed, the final CRC32 is crc32_update(...) ^ 0xFFFFFFFF.
[[gnu::section(".ewram")]] static u32 crc32_update(u32 crc, const u8* data, u32 len)
{
    for (u32 i = 0; i < len; i++)
        crc = gCrc32Table.Table((u8)((crc ^ data[i]) & 0xFF)) ^ (crc >> 8);
    return crc;
}

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sRomFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sUpsFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static FIL sWriteGpoFile;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static GpoHeader sCreateHeader;

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static u8 sUpsApplyBuf[4096];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static u8 sSourceChunkBuf[4096];

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
static u8 sXorOutBuf[4096];

[[gnu::section(".ewram")]] static bool readVlv(FIL* f, u32* outValue)
{
    VlvDecoder dec;
    while (true)
    {
        u8 b;
        UINT br;
        if (f_read(f, &b, 1, &br) != FR_OK || br != 1) return false;
        if (dec.Feed(b))
        {
            *outValue = dec.value;
            return true;
        }
    }
}

[[gnu::section(".ewram")]] static bool upsScanPass1(u32 patchedRomSize, u32 clusterSize, u32 recordsEndPos)
{
    u32 blocksPerCluster = clusterSize / 4096;
    if (blocksPerCluster == 0) blocksPerCluster = 1;

    u32 cursor = 0;
    while ((u32)f_tell(&sUpsFile) < recordsEndPos)
    {
        u32 delta;
        if (!readVlv(&sUpsFile, &delta)) return false;
        cursor += delta;
        if (cursor >= patchedRomSize) return false; // malformed: record starts past declared target size

        u32 recordStart = cursor;
        while (true)
        {
            u8 patchByte;
            UINT br;
            if (f_read(&sUpsFile, &patchByte, 1, &br) != FR_OK || br != 1) return false;
            cursor++;
            if (cursor > patchedRomSize) return false; // malformed: span exceeds declared target size
            if (patchByte == 0) break;
        }

        u32 spanLen = cursor - recordStart;
        u32 startBlock = recordStart / 4096;
        u32 endBlock   = (recordStart + spanLen - 1) / 4096;
        for (u32 b = startBlock; b <= endBlock; b++)
        {
            u32 c = b / blocksPerCluster;
            if (c < 8192)
                sCreateHeader.bitmask[c / 32] |= (1u << (c % 32));
        }
    }
    return true;
}

[[gnu::section(".ewram")]] static bool flushUpsChunk(u32 offset, const u8* patchBytes, u32 len, u32 romSize)
{
    u32 sourceAvailable = offset < romSize ? (romSize - offset) : 0;
    u32 sourceLen = len < sourceAvailable ? len : sourceAvailable;

    if (sourceLen > 0)
    {
        if (f_lseek(&sRomFile, offset) != FR_OK) return false;
        UINT br;
        if (f_read(&sRomFile, sSourceChunkBuf, sourceLen, &br) != FR_OK || br != sourceLen) return false;
    }

    for (u32 i = 0; i < len; i++)
    {
        u8 sourceByte = i < sourceLen ? sSourceChunkBuf[i] : 0;
        sXorOutBuf[i] = sourceByte ^ patchBytes[i];
    }

    return writePatchToGpo(&sWriteGpoFile, sCreateHeader, offset, sXorOutBuf, len);
}

[[gnu::section(".ewram")]] static bool upsApplyPass2(u32 romSize, u32 patchedRomSize, u32 recordsEndPos)
{
    u32 cursor = 0;
    while ((u32)f_tell(&sUpsFile) < recordsEndPos)
    {
        u32 delta;
        if (!readVlv(&sUpsFile, &delta)) return false;
        cursor += delta;
        if (cursor >= patchedRomSize) return false;

        u32 bufLen = 0;
        u32 flushStart = cursor;

        while (true)
        {
            u8 patchByte;
            UINT br;
            if (f_read(&sUpsFile, &patchByte, 1, &br) != FR_OK || br != 1) return false;

            sUpsApplyBuf[bufLen++] = patchByte;
            cursor++;
            if (cursor > patchedRomSize) return false;

            bool terminated = (patchByte == 0);
            if (bufLen == sizeof(sUpsApplyBuf) || terminated)
            {
                if (!flushUpsChunk(flushStart, sUpsApplyBuf, bufLen, romSize)) return false;
                flushStart += bufLen;
                bufLen = 0;
            }

            if (terminated) break;
        }
    }
    return true;
}

// Debug log codes for createGpoFromUps, val1=code, val2=context-dependent. All share one
// format string (the compiler folds repeated identical string literals into one .rodata
// entry) rather than ~25 distinct messages, since that .rodata still has to fit in the
// small `vrama` linker region shared with .text/.data/.bss on this target.
//  1 clusterSize<sizeof(GpoHeader)   2 upsSize too short        3 f_open rom failed
//  4 f_open ups failed               5 bad UPS1 magic           6 sourceSize VLV failed
//  7 targetSize VLV failed           8 declaredSourceSize       9 declaredTargetSize
// 10 declaredSourceSize!=romSize    11 recordsStartPos OOB      12 CRC footer read failed
// 13 expectedSourceCrc              14 expectedTargetCrc        15 expectedPatchCrc
// 16 source CRC32 read failed      17 computed source CRC32    18 source CRC32 mismatch
// 19 patch CRC32 read failed       20 computed patch CRC32     21 patch CRC32 mismatch
// 22 upsScanPass1 failed           23 upsScanPass1 OK           24 f_open gpo failed
// 25 header write failed           26 header padding failed    27 header+padding OK
// 28 copyRangeAClusters failed     29 writeRangeBExtension failed 30 Range A/B OK
// 31 upsApplyPass2 failed          32 upsApplyPass2 OK          33 Pass3 GPO read failed
// 34 Pass3 ROM read failed         35 computed target CRC32     36 target CRC32 mismatch
// 37 final magic-commit failed     38 completed successfully
#ifndef NDEBUG
#define UPS_LOG(romPath, code, val) log_debug(romPath, "createGpoFromUps step:", code, val)
#else
#define UPS_LOG(romPath, code, val) ((void)0)
#endif

[[gnu::section(".ewram")]] bool createGpoFromUps(const char* romPath, const char* upsPath, const char* gpoPath,
                                                    u32 romSize, u32 upsSize, u32 upsTimestamp, u32 clusterSize,
                                                    bool* outCrcMismatch)
{
    *outCrcMismatch = false;
    if (clusterSize < sizeof(GpoHeader))
    {
        UPS_LOG(romPath, 1, clusterSize);
        return false;
    }
    if (upsSize < 4 + 1 + 1 + 12)
    {
        UPS_LOG(romPath, 2, upsSize);
        return false;
    }

    if (f_open(&sRomFile, romPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
    {
        UPS_LOG(romPath, 3, 0);
        return false;
    }
    if (f_open(&sUpsFile, upsPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
    {
        UPS_LOG(romPath, 4, 0);
        f_close(&sRomFile);
        return false;
    }

    auto abortParse = [&]() -> bool {
        f_close(&sRomFile);
        f_close(&sUpsFile);
        return false;
    };

    char magic[4];
    UINT br;
    if (f_read(&sUpsFile, magic, 4, &br) != FR_OK || br != 4 || memcmp(magic, "UPS1", 4) != 0)
    {
        UPS_LOG(romPath, 5, 0);
        return abortParse();
    }

    u32 declaredSourceSize, declaredTargetSize;
    if (!readVlv(&sUpsFile, &declaredSourceSize))
    {
        UPS_LOG(romPath, 6, 0);
        return abortParse();
    }
    if (!readVlv(&sUpsFile, &declaredTargetSize))
    {
        UPS_LOG(romPath, 7, 0);
        return abortParse();
    }
    UPS_LOG(romPath, 8, declaredSourceSize);
    UPS_LOG(romPath, 9, declaredTargetSize);
    if (declaredSourceSize != romSize)
    {
        UPS_LOG(romPath, 10, declaredSourceSize);
        return abortParse();
    }

    u32 recordsStartPos = (u32)f_tell(&sUpsFile);
    if (upsSize < 12 || recordsStartPos > upsSize - 12)
    {
        UPS_LOG(romPath, 11, recordsStartPos);
        return abortParse();
    }
    u32 recordsEndPos = upsSize - 12;

    u32 expectedSourceCrc, expectedTargetCrc, expectedPatchCrc;
    if (f_lseek(&sUpsFile, recordsEndPos) != FR_OK) return abortParse();
    u8 crcFieldBuf[12];
    if (f_read(&sUpsFile, crcFieldBuf, 12, &br) != FR_OK || br != 12)
    {
        UPS_LOG(romPath, 12, 0);
        return abortParse();
    }
    expectedSourceCrc = crcFieldBuf[0] | (crcFieldBuf[1] << 8) | (crcFieldBuf[2] << 16) | ((u32)crcFieldBuf[3] << 24);
    expectedTargetCrc = crcFieldBuf[4] | (crcFieldBuf[5] << 8) | (crcFieldBuf[6] << 16) | ((u32)crcFieldBuf[7] << 24);
    expectedPatchCrc  = crcFieldBuf[8] | (crcFieldBuf[9] << 8) | (crcFieldBuf[10] << 16) | ((u32)crcFieldBuf[11] << 24);
    UPS_LOG(romPath, 13, expectedSourceCrc);
    UPS_LOG(romPath, 14, expectedTargetCrc);
    UPS_LOG(romPath, 15, expectedPatchCrc);

    u32 srcCrc = 0xFFFFFFFF;
    if (f_lseek(&sRomFile, 0) != FR_OK) return abortParse();
    {
        u32 remaining = romSize;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sUpsApplyBuf) ? (u32)sizeof(sUpsApplyBuf) : remaining;
            if (f_read(&sRomFile, sUpsApplyBuf, chunk, &br) != FR_OK || br != chunk)
            {
                UPS_LOG(romPath, 16, remaining);
                return abortParse();
            }
            srcCrc = crc32_update(srcCrc, sUpsApplyBuf, chunk);
            remaining -= chunk;
        }
    }
    srcCrc ^= 0xFFFFFFFF;
    UPS_LOG(romPath, 17, srcCrc);
    if (srcCrc != expectedSourceCrc) { UPS_LOG(romPath, 18, srcCrc); *outCrcMismatch = true; return abortParse(); }

    u32 patchCrc = 0xFFFFFFFF;
    if (f_lseek(&sUpsFile, 0) != FR_OK) return abortParse();
    {
        u32 remaining = upsSize - 4;
        while (remaining > 0)
        {
            u32 chunk = remaining > sizeof(sUpsApplyBuf) ? (u32)sizeof(sUpsApplyBuf) : remaining;
            if (f_read(&sUpsFile, sUpsApplyBuf, chunk, &br) != FR_OK || br != chunk)
            {
                UPS_LOG(romPath, 19, remaining);
                return abortParse();
            }
            patchCrc = crc32_update(patchCrc, sUpsApplyBuf, chunk);
            remaining -= chunk;
        }
    }
    patchCrc ^= 0xFFFFFFFF;
    UPS_LOG(romPath, 20, patchCrc);
    if (patchCrc != expectedPatchCrc) { UPS_LOG(romPath, 21, patchCrc); *outCrcMismatch = true; return abortParse(); }

    memset(&sCreateHeader, 0, sizeof(sCreateHeader));
    if (f_lseek(&sUpsFile, recordsStartPos) != FR_OK) return abortParse();
    // No bitmask-cleanup pass needed here unlike IPS: upsScanPass1's `cursor >= patchedRomSize`
    // guard already prevents any bit at or past patchedTotalClusters from ever being set.
    if (!upsScanPass1(declaredTargetSize, clusterSize, recordsEndPos))
    {
        UPS_LOG(romPath, 22, 0);
        return abortParse();
    }
    UPS_LOG(romPath, 23, 0);

    u32 patchedRomSize = declaredTargetSize;
    memcpy(sCreateHeader.magic, GPO_MAGIC_PLACEHOLDER, 4);
    sCreateHeader.romSize = romSize;
    sCreateHeader.patchedRomSize = patchedRomSize;
    sCreateHeader.patchSize = upsSize;
    sCreateHeader.patchTimestamp = upsTimestamp;
    sCreateHeader.patchKind = (u8)GpoPatchKind::Ups;
    sCreateHeader.clusterSize = clusterSize;

    // FA_READ is required (not just FA_WRITE) because Pass 3 below reads this same handle
    // back to verify the target CRC32 -- FatFS enforces access-mode flags strictly, so a
    // write-only handle would reject every f_read() in that pass.
    if (f_open(&sWriteGpoFile, gpoPath, FA_READ | FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    {
        UPS_LOG(romPath, 24, 0);
        return abortParse();
    }

    auto abortBuild = [&]() -> bool {
        f_close(&sRomFile);
        f_close(&sUpsFile);
        f_close(&sWriteGpoFile);
        f_unlink(gpoPath);
        return false;
    };

    UINT bw;
    if (f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw) != FR_OK || bw != sizeof(sCreateHeader))
    {
        UPS_LOG(romPath, 25, bw);
        return abortBuild();
    }
    {
        u32 paddingBytes = clusterSize - sizeof(sCreateHeader);
        memset(sUpsApplyBuf, 0, sizeof(sUpsApplyBuf));
        while (paddingBytes > 0)
        {
            u32 chunk = paddingBytes > sizeof(sUpsApplyBuf) ? (u32)sizeof(sUpsApplyBuf) : paddingBytes;
            if (f_write(&sWriteGpoFile, sUpsApplyBuf, chunk, &bw) != FR_OK || bw != chunk)
            {
                UPS_LOG(romPath, 26, bw);
                return abortBuild();
            }
            paddingBytes -= chunk;
        }
    }
    UPS_LOG(romPath, 27, 0);

    if (!copyRangeAClusters(&sWriteGpoFile, &sRomFile, sCreateHeader))
    {
        UPS_LOG(romPath, 28, 0);
        return abortBuild();
    }
    if (!writeRangeBExtensionClusters(&sWriteGpoFile, sCreateHeader))
    {
        UPS_LOG(romPath, 29, 0);
        return abortBuild();
    }
    UPS_LOG(romPath, 30, 0);

    if (f_lseek(&sUpsFile, recordsStartPos) != FR_OK) return abortBuild();
    if (!upsApplyPass2(romSize, patchedRomSize, recordsEndPos))
    {
        UPS_LOG(romPath, 31, 0);
        return abortBuild();
    }
    UPS_LOG(romPath, 32, 0);

    u32 targetCrc = 0xFFFFFFFF;
    {
        u32 patchedTotalClusters = (patchedRomSize + clusterSize - 1) / clusterSize;
        for (u32 c = 0; c < patchedTotalClusters; c++)
        {
            u32 gpoOffset;
            bool fromGpo = gpoClusterFileOffset(sCreateHeader, c, &gpoOffset);

            u32 bytesInCluster = clusterSize;
            if (c == patchedTotalClusters - 1)
            {
                u32 used = patchedRomSize - c * clusterSize;
                if (used < clusterSize) bytesInCluster = used;
            }

            u32 remaining = bytesInCluster;
            u32 byteOffsetInCluster = 0;
            // For the unpatched-ROM-cluster branch, the cluster's bytes past
            // romSize don't exist in sRomFile. Clamp the real read to the
            // in-bounds portion and feed implicit zero bytes for the rest,
            // matching flushUpsChunk's implicit-zero convention.
            u32 realBytesInCluster = bytesInCluster;
            if (!fromGpo)
            {
                u32 clusterStart = c * clusterSize;
                u32 romAvailable = clusterStart < romSize ? (romSize - clusterStart) : 0;
                if (romAvailable < realBytesInCluster) realBytesInCluster = romAvailable;
            }
            while (remaining > 0)
            {
                u32 chunk = remaining > sizeof(sUpsApplyBuf) ? (u32)sizeof(sUpsApplyBuf) : remaining;
                if (fromGpo)
                {
                    if (f_lseek(&sWriteGpoFile, gpoOffset + byteOffsetInCluster) != FR_OK) return abortBuild();
                    if (f_read(&sWriteGpoFile, sUpsApplyBuf, chunk, &br) != FR_OK || br != chunk)
                    {
                        UPS_LOG(romPath, 33, c);
                        return abortBuild();
                    }
                }
                else if (byteOffsetInCluster < realBytesInCluster)
                {
                    // Real bytes remain in this chunk; clamp this read to not
                    // cross past realBytesInCluster, zero-fill the rest below.
                    u32 realChunk = chunk;
                    if (byteOffsetInCluster + realChunk > realBytesInCluster)
                        realChunk = realBytesInCluster - byteOffsetInCluster;
                    if (f_lseek(&sRomFile, (FSIZE_t)c * clusterSize + byteOffsetInCluster) != FR_OK) return abortBuild();
                    if (f_read(&sRomFile, sUpsApplyBuf, realChunk, &br) != FR_OK || br != realChunk)
                    {
                        UPS_LOG(romPath, 34, c);
                        return abortBuild();
                    }
                    if (realChunk < chunk) memset(sUpsApplyBuf + realChunk, 0, chunk - realChunk);
                }
                else
                {
                    // Entirely past romSize: implicit zero bytes, no read.
                    memset(sUpsApplyBuf, 0, chunk);
                }
                targetCrc = crc32_update(targetCrc, sUpsApplyBuf, chunk);
                byteOffsetInCluster += chunk;
                remaining -= chunk;
            }
        }
    }
    targetCrc ^= 0xFFFFFFFF;
    UPS_LOG(romPath, 35, targetCrc);
    if (targetCrc != expectedTargetCrc) { UPS_LOG(romPath, 36, targetCrc); *outCrcMismatch = true; return abortBuild(); }

    memcpy(sCreateHeader.magic, GPO_MAGIC, 4);
    if (f_lseek(&sWriteGpoFile, 0) != FR_OK) return abortBuild();
    if (f_write(&sWriteGpoFile, &sCreateHeader, sizeof(sCreateHeader), &bw) != FR_OK
        || bw != sizeof(sCreateHeader))
    {
        UPS_LOG(romPath, 37, bw);
        return abortBuild();
    }
    UPS_LOG(romPath, 38, 0);

    f_close(&sRomFile);
    f_close(&sUpsFile);
    f_close(&sWriteGpoFile);
    return true;
}
