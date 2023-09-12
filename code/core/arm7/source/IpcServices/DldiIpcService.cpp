#include "common.h"
#include <string.h>
#include "dldi.h"
#include "DldiIpcService.h"

void DldiIpcService::HandleMessage(u32 data)
{
    auto cmd = reinterpret_cast<const dldi_ipc_cmd_t*>(data << 2);
    switch (cmd->cmd)
    {
        case DLDI_IPC_CMD_SETUP:
        {
            SetupDldi(cmd);
            break;
        }
        case DLDI_IPC_CMD_READ_SECTORS:
        {
            ReadSectors(cmd);
            break;
        }
        case DLDI_IPC_CMD_WRITE_SECTORS:
        {
            WriteSectors(cmd);
            break;
        }
    }
}

void DldiIpcService::SetupDldi(const dldi_ipc_cmd_t* cmd) const
{
    memcpy(_dldi_start, cmd->buffer, sizeof(_dldi_start));
    bool result = _DLDI_startup_ptr();
    SendResponseMessage(result);
}

void DldiIpcService::ReadSectors(const dldi_ipc_cmd_t* cmd) const
{
    _DLDI_readSectors_ptr(cmd->sector, cmd->count, cmd->buffer);
    SendResponseMessage(0);
}

void DldiIpcService::WriteSectors(const dldi_ipc_cmd_t* cmd) const
{
    _DLDI_writeSectors_ptr(cmd->sector, cmd->count, cmd->buffer);
    SendResponseMessage(0);
}
