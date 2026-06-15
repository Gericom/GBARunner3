#pragma once
#include "common.h"
#include "Fat/ff.h"

#define GPO_MAGIC "GPO1"
#define GPO_MAGIC_PLACEHOLDER "GPOT"

struct GpoHeader
{
    char magic[4];
    u32 romSize;
    u32 patchedRomSize;      // effective ROM size after patch; 0 means same as romSize
    u32 ipsSize;
    u32 ipsTimestamp;
    u32 clusterSize;
    u32 bitmask[8192 / 32];
};

extern FIL gGpoFile;
extern u32 gGpoBitmask[8192 / 32];

#ifdef __cplusplus
extern "C" {
#endif

bool gpo_init(const char* romPath);
void gpo_patchLinearChunk(u32 clusterSize, u32 linearChunkSize);

#ifdef __cplusplus
}
#endif
