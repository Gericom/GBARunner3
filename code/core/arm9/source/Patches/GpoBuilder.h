#pragma once
#include "common.h"
#include "Fat/ff.h"
#include "GpoPatcher.h"

void gpo_getPath(const char* romPath, char* outPath, const char* ext);
void log_debug(const char* romPath, const char* msg, u32 val1 = 0xFFFFFFFF, u32 val2 = 0xFFFFFFFF);
void log_table(const char* romPath, const DWORD* tbl);

// True if ROM cluster `cluster` is stored in the GPO file (patched or extension cluster);
// *outOffset is then its byte offset within the GPO file. False means: read this cluster
// unmodified from the original ROM at `cluster * header.clusterSize` instead.
bool gpoClusterFileOffset(const GpoHeader& header, u32 cluster, u32* outOffset);

bool writePatchToGpo(FIL* gpoFile, const GpoHeader& header, u32 romOffset, const u8* data, u32 dataSize);

bool verifyGpo(const char* gpoPath, u32 romSize, u32 patchSize, u32 patchTimestamp,
               u8 patchKind, u32 clusterSize);

bool copyRangeAClusters(FIL* gpoFile, FIL* romFile, const GpoHeader& header);
bool writeRangeBExtensionClusters(FIL* gpoFile, const GpoHeader& header);

bool createGpoFromIps(const char* romPath, const char* ipsPath, const char* gpoPath,
                       u32 romSize, u32 ipsSize, u32 ipsTime, u32 clusterSize);
