#pragma once

typedef enum
{
    FS_IPC_CMD_DLDI_SETUP,
    FS_IPC_CMD_DLDI_READ_SECTORS,
    FS_IPC_CMD_DLDI_WRITE_SECTORS,
    FS_IPC_CMD_DSI_SD_READ_SECTORS,
    FS_IPC_CMD_DSI_SD_WRITE_SECTORS
} FsIpcCommand;

typedef struct alignas(32)
{
    u32 cmd;
    void* buffer;
    u32 sector;
    u32 count;
} fs_ipc_cmd_t;
