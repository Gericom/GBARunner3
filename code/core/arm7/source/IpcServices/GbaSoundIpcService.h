#pragma once
#include "ThreadIpcService.h"
#include "GbaSoundIpcCommand.h"
#include "IpcChannels.h"

class GbaSoundIpcService : public IpcService
{
public:
    GbaSoundIpcService()
        : IpcService(IPC_CHANNEL_GBA_SOUND) { }

    void OnMessageReceived(u32 data) override;
};
