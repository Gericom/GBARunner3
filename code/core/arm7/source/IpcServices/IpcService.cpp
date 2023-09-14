#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include "IpcService.h"

void IpcService::Start()
{
    ipc_setChannelHandler(_ipcChannel, [] (u32 channel, u32 data, void* arg)
    {
        static_cast<IpcService*>(arg)->OnMessageReceived(data);
    }, this);
}
