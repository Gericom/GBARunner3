#include "common.h"
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include "SystemIpcCommand.h"
#include "IpcChannels.h"
#include "SystemIpc.h"

static void sendSystemIpcMessage(SystemIpcCommand command, u32 data)
{
    ipc_sendWordDirect(
        (data << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
        (command << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_SYSTEM);
}

void sysipc_setTopBacklight(bool enabled)
{
    sendSystemIpcMessage(SYSTEM_IPC_CMD_SET_TOP_BACKLIGHT, enabled ? 1 : 0);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

void sysipc_setBottomBacklight(bool enabled)
{
    sendSystemIpcMessage(SYSTEM_IPC_CMD_SET_BOTTOM_BACKLIGHT, enabled ? 1 : 0);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}
