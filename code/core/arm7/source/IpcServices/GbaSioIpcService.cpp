#include "common.h"
#include "GbaSio.h"
#include "GbaSioDefinitions.h"
#include "GbaSioIpcCommand.h"
#include "Rfu/Rfu.h"
#include "GbaSioIpcService.h"

// void GbaSioIpcService::Start()
// {
//     rtos_createEvent(&_event);
//     rtos_createThread(&_thread, 8, [] (void* arg)
//     {
//         static_cast<GbaSioIpcService*>(arg)->ThreadMain();
//     }, this, _stack, sizeof(_stack));
//     rtos_wakeupThread(&_thread);
//     IpcService::Start();
// }

// void GbaSioIpcService::ThreadMain()
// {
//     while (true)
//     {
//         rtos_waitEvent(&_event, false, true);
//         u32 readPtr = _messageRingReadPtr;
//         while (readPtr != _messageRingWritePtr)
//         {
//             HandleMessage(_messageRing[readPtr]);
//             readPtr = (readPtr + 1) % SIO_IPC_MESSAGE_RING_COUNT;
//             _messageRingReadPtr = readPtr;
//         }
//     }
// }

// void GbaSioIpcService::HandleMessage(u32 data)
// {
//     auto cmd = static_cast<GbaSioIpcCommand>(data & 0x7);
//     switch (cmd)
//     {
//         case GBA_SIO_IPC_CMD_SETUP:
//         {
//             auto sharedData = reinterpret_cast<sio_shared_t*>((data >> 3) << 5);
//             rfu_init(sharedData);
//             SendResponseMessage(0);
//             break;
//         }
//         case GBA_SIO_IPC_CMD_RFU_RESET:
//         {
//             rfu_reset();
//             break;
//         }
//         case GBA_SIO_IPC_CMD_RFU_TRANSFER:
//         {
//             u32 sioCnt = data >> 3;
//             rfu_doTransfer(sioCnt & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL);
//             break;
//         }
//         case GBA_SIO_IPC_CMD_SO_CHANGED:
//         {
//             rfu_soChanged();
//             break;
//         }
//     }
// }

void GbaSioIpcService::OnMessageReceived(u32 data)
{
    // while (((_messageRingWritePtr + 1) % SIO_IPC_MESSAGE_RING_COUNT) == _messageRingReadPtr);
    // _messageRing[_messageRingWritePtr] = data;
    // _messageRingWritePtr = (_messageRingWritePtr + 1) % SIO_IPC_MESSAGE_RING_COUNT;
    // rtos_signalEvent(&_event);
    auto cmd = static_cast<GbaSioIpcCommand>(data & 0x7);
    switch (cmd)
    {
        case GBA_SIO_IPC_CMD_SETUP:
        {
            auto sharedData = reinterpret_cast<sio_shared_t*>((data >> 3) << 5);
            rfu_init(sharedData);
            SendResponseMessage(0);
            break;
        }
        case GBA_SIO_IPC_CMD_RFU_RESET:
        {
            rfu_reset();
            break;
        }
        case GBA_SIO_IPC_CMD_RFU_TRANSFER:
        {
            u32 sioCnt = data >> 3;
            rfu_doTransfer(sioCnt & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL);
            break;
        }
        case GBA_SIO_IPC_CMD_SO_CHANGED:
        {
            rfu_soChanged();
            break;
        }
    }
}
