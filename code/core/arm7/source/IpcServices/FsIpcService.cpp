#include "common.h"
#include <string.h>
#include "dldi.h"
#include "FsIpcService.h"

extern "C" int sdmmc_sd_startup();

void FsIpcService::Start()
{
    if (isDSiMode())
    {
        sdmmc_sd_startup();
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
    SendResponseMessage(0);
}

void FsIpcService::DldiWriteSectors(const fs_ipc_cmd_t* cmd) const
{
    _DLDI_writeSectors_ptr(cmd->sector, cmd->count, cmd->buffer);
    SendResponseMessage(0);
}

void FsIpcService::DsiSdReadSectors(const fs_ipc_cmd_t* cmd) const
{
    sdmmc_sdcard_readsectors(cmd->sector, cmd->count, cmd->buffer);
    SendResponseMessage(0);
}

void FsIpcService::DsiSdWriteSectors(const fs_ipc_cmd_t* cmd) const
{
    sdmmc_sdcard_writesectors(cmd->sector, cmd->count, cmd->buffer);
    SendResponseMessage(0);
}
