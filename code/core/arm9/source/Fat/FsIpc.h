#pragma once

typedef enum
{
    FS_DEVICE_DLDI,
    FS_DEVICE_DSI_SD
} FsDevice;

#ifdef __cplusplus
extern "C" {
#endif

void fs_readSectors(FsDevice device, void* buffer, u32 sector, u32 count);
void fs_writeSectors(FsDevice device, const void* buffer, u32 sector, u32 count);

#ifdef __cplusplus
}
#endif
