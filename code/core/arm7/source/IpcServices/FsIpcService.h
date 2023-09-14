#pragma once
#include "ThreadIpcService.h"
#include "FsIpcCommand.h"
#include "IpcChannels.h"

class FsIpcService : public ThreadIpcService
{
    u32 _threadStack[128];

    void SetupDldi(const fs_ipc_cmd_t* cmd) const;
    void DldiReadSectors(const fs_ipc_cmd_t* cmd) const;
    void DldiWriteSectors(const fs_ipc_cmd_t* cmd) const;
    void DsiSdReadSectors(const fs_ipc_cmd_t* cmd) const;
    void DsiSdWriteSectors(const fs_ipc_cmd_t* cmd) const;

public:
    FsIpcService()
        : ThreadIpcService(IPC_CHANNEL_FS, 6, _threadStack, sizeof(_threadStack)) { }

    void Start() override;
    void HandleMessage(u32 data) override;
};
