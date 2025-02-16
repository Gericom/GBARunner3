#include "common.h"
#include <libtwl/ipc/ipcSync.h>
#include "../mmc/sdmmc.h"
#include <string.h>
#include "dldi.h"
#include "FsIpcService.h"

void FsIpcService::Start()
{
    if (isDSiMode())
    {
        _SDMMC_init(SDMMC_DEV_CARD);
    }
    ThreadIpcService::Start();
}

void FsIpcService::HandleMessage(u32 data)
{
    auto cmd = reinterpret_cast<const fs_ipc_cmd_t*>(data << 2);
    switch (cmd->cmd)
    {
        case FS_IPC_CMD_DLDI_SETUP:
        {
            SetupDldi(cmd);
            break;
        }
        case FS_IPC_CMD_DLDI_READ_SECTORS:
        {
            DldiReadSectors(cmd);
            break;
        }
        case FS_IPC_CMD_DLDI_WRITE_SECTORS:
        {
            DldiWriteSectors(cmd);
            break;
        }
        case FS_IPC_CMD_DSI_SD_READ_SECTORS:
        {
            DsiSdReadSectors(cmd);
            break;
        }
        case FS_IPC_CMD_DSI_SD_WRITE_SECTORS:
        {
            DsiSdWriteSectors(cmd);
            break;
        }
    }
}

void FsIpcService::SetupDldi(const fs_ipc_cmd_t* cmd) const
{
    memcpy(_dldi_start, cmd->buffer, sizeof(_dldi_start));
    bool result = _DLDI_startup_ptr();
    SendResponseMessage(result);
}

void FsIpcService::DldiReadSectors(const fs_ipc_cmd_t* cmd) const
{
    _DLDI_readSectors_ptr(cmd->sector, cmd->count, cmd->buffer);
    ipc_setArm7SyncBits(ipc_getArm9SyncBits());
}

void FsIpcService::DldiWriteSectors(const fs_ipc_cmd_t* cmd) const
{
    _DLDI_writeSectors_ptr(cmd->sector, cmd->count, cmd->buffer);
    ipc_setArm7SyncBits(ipc_getArm9SyncBits());
}

void FsIpcService::DsiSdReadSectors(const fs_ipc_cmd_t* cmd) const
{
    _SDMMC_readSectors(SDMMC_DEV_CARD, cmd->sector, cmd->buffer, cmd->count);
    ipc_setArm7SyncBits(ipc_getArm9SyncBits());
}

void FsIpcService::DsiSdWriteSectors(const fs_ipc_cmd_t* cmd) const
{
    _SDMMC_writeSectors(SDMMC_DEV_CARD, cmd->sector, cmd->buffer, cmd->count);
    ipc_setArm7SyncBits(ipc_getArm9SyncBits());
}
