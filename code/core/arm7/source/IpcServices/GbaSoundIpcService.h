#pragma once
#include "ThreadIpcService.h"
#include "GbaSoundIpcCommand.h"
#include "IpcChannels.h"

class GbaSoundIpcService : public ThreadIpcService
{
    u32 _threadStack[128];

public:
    GbaSoundIpcService()
        : ThreadIpcService(IPC_CHANNEL_GBA_SOUND, 4, _threadStack, sizeof(_threadStack)) { }

    void HandleMessage(u32 data) override;
};
