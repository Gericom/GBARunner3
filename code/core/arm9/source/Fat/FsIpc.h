#pragma once

typedef enum
{
    FS_DEVICE_DLDI,
    FS_DEVICE_DSI_SD
} FsDevice;

/// @brief Struct used to track completion of an async sd read or write.
typedef struct
{
    vu16 transactionComplete;
} FsWaitToken;

#ifdef __cplusplus
extern "C" {
#endif

void fs_readSectors(FsDevice device, void* buffer, u32 sector, u32 count);
void fs_writeSectors(FsDevice device, const void* buffer, u32 sector, u32 count);

void fs_readCacheAlignedSectorsAsync(FsDevice device, void* buffer, u32 sector, u32 count, FsWaitToken* waitToken);
void fs_writeCacheAlignedSectorsAsync(FsDevice device, const void* buffer, u32 sector, u32 count, FsWaitToken* waitToken);
u32 fs_waitForCompletion(FsWaitToken* waitToken, bool keepIrqsDisabled);
u32 fs_waitForCompletionOfCurrentTransaction(bool keepIrqsDisabled);

#ifdef __cplusplus
}
#endif
