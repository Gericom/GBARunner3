#pragma once

typedef enum
{
    DLDI_IPC_CMD_SETUP,
    DLDI_IPC_CMD_READ_SECTORS,
    DLDI_IPC_CMD_WRITE_SECTORS
} DldiIpcCommand;

typedef struct alignas(32)
{
    u32 cmd;
    void* buffer;
    u32 sector;
    u32 count;
} dldi_ipc_cmd_t;