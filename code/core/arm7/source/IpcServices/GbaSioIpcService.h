#pragma once
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosEvent.h>
#include "IpcService.h"
#include "GbaSioIpcCommand.h"
#include "IpcChannels.h"

#define SIO_IPC_MESSAGE_RING_COUNT  32

class GbaSioIpcService : public IpcService
{
    // rtos_thread_t _thread;
    // rtos_event_t _event;
    // u32 _stack[4096 / 4];
    // u32 _messageRing[SIO_IPC_MESSAGE_RING_COUNT];
    // vu32 _messageRingReadPtr = 0;
    // vu32 _messageRingWritePtr = 0;

    // void ThreadMain();
    // void HandleMessage(u32 data);
public:
    GbaSioIpcService()
        : IpcService(IPC_CHANNEL_GBA_SIO) { }

    // void Start() override;
    void OnMessageReceived(u32 data) override;
};
