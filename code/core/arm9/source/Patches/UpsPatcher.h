#pragma once
#include "common.h"
#include "Fat/ff.h"
#include "GpoPatcher.h"

struct VlvDecoder
{
    u32 value = 0;
    u32 shift = 1;

    // Feed one byte; returns true once this VLV value is complete.
    bool Feed(u8 byte)
    {
        value += (u32)(byte & 0x7F) * shift;
        if (byte & 0x80) return true;
        shift <<= 7;
        value += shift;
        return false;
    }
};

// outCrcMismatch is set true only when a fully-read CRC32 (source, target, or patch)
// disagrees with the .ups header; any I/O or parse failure leaves it false.
bool createGpoFromUps(const char* romPath, const char* upsPath, const char* gpoPath,
                       u32 romSize, u32 upsSize, u32 upsTimestamp, u32 clusterSize,
                       bool* outCrcMismatch);
