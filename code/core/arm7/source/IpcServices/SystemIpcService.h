#pragma once
#include "ThreadIpcService.h"
#include "SystemIpcCommand.h"
#include "IpcChannels.h"

class SystemIpcService : public ThreadIpcService
{
    u32 _threadStack[128];

public:
    SystemIpcService()
        : ThreadIpcService(IPC_CHANNEL_SYSTEM, 4, _threadStack, sizeof(_threadStack)) { }

    void HandleMessage(u32 data) override;
};
