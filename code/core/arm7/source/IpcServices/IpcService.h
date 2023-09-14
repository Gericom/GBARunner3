#pragma once
#include <libtwl/ipc/ipcFifoSystem.h>

/// @brief Abstract base class for IPC services.
class IpcService
{
    const u32 _ipcChannel;
protected:
    /// @brief Constructs the IPC service with the given channel number.
    /// @param ipcChannel The channel number this IPC service will use.
    explicit IpcService(u32 ipcChannel)
        : _ipcChannel(ipcChannel) { }

    /// @brief Sends back a response message to the other CPU.
    /// @param data The data bits of the response message.
    void SendResponseMessage(u32 data) const
    {
        ipc_sendFifoMessage(_ipcChannel, data);
    }

public:
    /// @brief Initializes the IPC service and registers the channel handler.
    virtual void Start();

    /// @brief Method for handling received messages.
    /// @param data The data bits from the received message.
    virtual void OnMessageReceived(u32 data) = 0;
};
