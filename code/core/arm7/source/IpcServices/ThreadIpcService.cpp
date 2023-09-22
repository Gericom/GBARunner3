#include "common.h"
#include "ThreadIpcService.h"

void ThreadIpcService::ThreadMain()
{
    while (true)
    {
        rtos_waitEvent(&_event, false, true);
        if (_messageValid)
        {
            _messageValid = false;
            HandleMessage(_message);
        }
    }
}

void ThreadIpcService::Start()
{
    rtos_createEvent(&_event);
    rtos_createThread(&_thread, _priority, [] (void* arg)
    {
        static_cast<ThreadIpcService*>(arg)->ThreadMain();
    }, this, _stack, _stackSize);
    rtos_wakeupThread(&_thread);
    IpcService::Start();
}

void ThreadIpcService::OnMessageReceived(u32 data)
{
    _message = data;
    _messageValid = true;
    rtos_signalEvent(&_event);
}
