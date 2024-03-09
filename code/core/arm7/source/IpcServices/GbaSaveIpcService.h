#pragma once
#include "ThreadIpcService.h"
#include "GbaSaveShared.h"
#include "GbaSaveIpcCommand.h"
#include "IpcChannels.h"

class GbaSaveIpcService : public IpcService
{
    gba_save_shared_t* _saveShared = nullptr;
    u32 _saveWaitCounter;

public:
    GbaSaveIpcService()
        : IpcService(IPC_CHANNEL_GBA_SAVE) { }

    void OnMessageReceived(u32 data) override;

    void Update();
    bool FlushSaveIfDirty();
};
