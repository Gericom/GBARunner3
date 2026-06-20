#pragma once
#include "common.h"
#include "Fat/ff.h"

#define GPO_MAGIC "GPO2"
#define GPO_MAGIC_PLACEHOLDER "GPOT"

enum class GpoPatchKind : u8 { Ips = 0, Ups = 1 };

struct GpoHeader
{
    char magic[4];
    u32  romSize;
    u32  patchedRomSize;      // effective ROM size after patch; 0 means same as romSize
    u32  patchSize;           // was ipsSize
    u32  patchTimestamp;      // was ipsTimestamp
    u8   patchKind;           // GpoPatchKind, stored as u8 for a stable on-disk layout
    u32  clusterSize;
    u32  bitmask[8192 / 32];
};

enum class GpoInitResult { Inactive, Active, FatalMismatch };

extern FIL gGpoFile;
extern u32 gGpoBitmask[8192 / 32];

#ifdef __cplusplus
extern "C" {
#endif

GpoInitResult gpo_init(const char* romPath);
void gpo_patchLinearChunk(u32 clusterSize, u32 linearChunkSize);

#ifdef __cplusplus
}
#endif
