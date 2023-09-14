#pragma once
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosEvent.h>
#include "IpcService.h"

/// @brief Base class for IPC services with a thread that performs the actual
///        message processing. This makes it possible to prioritize messages
///        of different IPC services and prevents blocking IRQs.
class ThreadIpcService : public IpcService
{
    rtos_thread_t _thread;
    rtos_event_t _event;
    u32* _stack;
    u32 _stackSize;
    u8 _priority;
    bool _messageValid = false;
    u32 _message;

    void ThreadMain();

protected:
    /// @brief Method for handling received messages on the thread.
    /// @param data The data bits from the received message.
    virtual void HandleMessage(u32 data) = 0;

public:
    ThreadIpcService(u32 ipcChannel, u8 priority, u32* stack, u32 stackSize)
        : IpcService(ipcChannel), _stack(stack), _stackSize(stackSize), _priority(priority) { }

    /// @brief Initializes the IPC service and registers the channel handler.
    void Start() override;

    /// @brief Method for handling received messages.
    /// @param data The data bits from the received message.
    void OnMessageReceived(u32 data) override;
};
