#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include <string.h>
#include "IpcChannels.h"
#include "FsIpcCommand.h"
#include "cp15.h"
#include "Cpsr.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "FsIpc.h"

[[gnu::section(".ewram.bss")]]
alignas(32) static fs_ipc_cmd_t sIpcCommand;
[[gnu::section(".ewram.bss")]]
alignas(32) static u8 sTempBuffers[2][512];

static FsWaitToken* volatile sCurrentWaitToken = nullptr;

static bool isArm7FsOperationComplete()
{
    u32 ipcSync = REG_IPCSYNC;
    return ((ipcSync >> IPCSYNC_LOCAL_DATA_SHIFT) & 0xF) == (ipcSync & IPCSYNC_REMOTE_DATA_MASK);
}

extern "C"
[[gnu::noinline]]
u32 fs_waitForCompletion(FsWaitToken* waitToken, bool keepIrqsDisabled)
{
    u32 irqs;
#ifdef GBAR3_IRQ_YIELDING
    if (gIrqYieldingEnabled)
    {
        irqs = arm_disableIrqs();
        vm_disableIrqYielding();
        while (true)
        {
            if (waitToken && waitToken->transactionComplete)
            {
                break;
            }
            FsWaitToken* currentWaitToken = sCurrentWaitToken;
            if (!currentWaitToken)
            {
                break;
            }
            else if (isArm7FsOperationComplete())
            {
                currentWaitToken->transactionComplete = true;
                sCurrentWaitToken = nullptr;
                break;
            }
            if (!(irqs & 0x80) && !vm_yieldGbaIrqs())
            {
                arm_restoreIrqs(irqs);
                irqs = arm_disableIrqs();
            }
        }
        vm_restoreIrqYielding(true);
    }
    else
#endif
    {
        while (true)
        {
            irqs = arm_disableIrqs();
            if (waitToken && waitToken->transactionComplete)
            {
                break;
            }
            FsWaitToken* currentWaitToken = sCurrentWaitToken;
            if (!currentWaitToken)
            {
                break;
            }
            else if (isArm7FsOperationComplete())
            {
                currentWaitToken->transactionComplete = true;
                sCurrentWaitToken = nullptr;
                break;
            }
            arm_restoreIrqs(irqs);
        }
    }

    if (!keepIrqsDisabled)
    {
        arm_restoreIrqs(irqs);
    }

    return irqs;
}

extern "C" u32 fs_waitForCompletionOfCurrentTransaction(bool keepIrqsDisabled)
{
    return fs_waitForCompletion(nullptr, keepIrqsDisabled);
}

static void sendIpcCommand()
{
    dc_flushRange(&sIpcCommand, sizeof(sIpcCommand));
    ipc_setArm9SyncBits(ipc_getArm7SyncBits() + 1);
    ipc_sendWordDirect(((((u32)&sIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_FS);
}

static void executeIpcCommandAsync(u32 cmd, void* buffer, u32 sector, u32 count, FsWaitToken* waitToken)
{
    waitToken->transactionComplete = false;
    u32 irqs = fs_waitForCompletion(nullptr, true);
    {
        sIpcCommand.cmd = cmd;
        sIpcCommand.buffer = buffer;
        sIpcCommand.sector = sector;
        sIpcCommand.count = count;
        sendIpcCommand();
        sCurrentWaitToken = waitToken;
    }
    arm_restoreIrqs(irqs);
}

extern "C" void fs_readCacheAlignedSectorsAsync(FsDevice device, void* buffer, u32 sector, u32 count, FsWaitToken* waitToken)
{
    dc_invalidateRange(buffer, 512 * count);
    executeIpcCommandAsync(
        device == FS_DEVICE_DLDI ? FS_IPC_CMD_DLDI_READ_SECTORS : FS_IPC_CMD_DSI_SD_READ_SECTORS,
        buffer, sector, count, waitToken);
}

static void readSectorsNotCacheAligned(FsDevice device, void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = device == FS_DEVICE_DLDI ? FS_IPC_CMD_DLDI_READ_SECTORS : FS_IPC_CMD_DSI_SD_READ_SECTORS;
    sIpcCommand.count = 1;
    // not cache aligned, use a temp buffer
    dc_invalidateRange(sTempBuffers[0], 512);
    for (u32 i = 0; i < count; i++)
    {
        sIpcCommand.buffer = sTempBuffers[i & 1];
        sIpcCommand.sector = sector + i;
        sendIpcCommand();
        if (i != 0)
            memcpy((u8*)buffer + 512 * (i - 1), sTempBuffers[(i - 1) & 1], 512);
        if (i != count - 1)
            dc_invalidateRange(sTempBuffers[(i + 1) & 1], 512);
        while (!isArm7FsOperationComplete());
    }
    memcpy((u8*)buffer + 512 * (count - 1), sTempBuffers[(count - 1) & 1], 512);
}

extern "C" void fs_readSectors(FsDevice device, void* buffer, u32 sector, u32 count)
{
    if (count == 0)
        return;

    if (((u32)buffer >> 24) != 2 || ((u32)buffer & 0x1F))
    {
        readSectorsNotCacheAligned(device, buffer, sector, count);
    }
    else
    {
        FsWaitToken waitToken;
        fs_readCacheAlignedSectorsAsync(device, buffer, sector, count, &waitToken);
        fs_waitForCompletion(&waitToken, false);
    }
}

extern "C" void fs_writeCacheAlignedSectorsAsync(FsDevice device, const void* buffer, u32 sector, u32 count, FsWaitToken* waitToken)
{
    dc_flushRange(buffer, 512 * count);
    executeIpcCommandAsync(
        device == FS_DEVICE_DLDI ? FS_IPC_CMD_DLDI_WRITE_SECTORS : FS_IPC_CMD_DSI_SD_WRITE_SECTORS,
        (void*)buffer, sector, count, waitToken);
}

static void writeSectorsNotCacheAligned(FsDevice device, const void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = device == FS_DEVICE_DLDI ? FS_IPC_CMD_DLDI_WRITE_SECTORS : FS_IPC_CMD_DSI_SD_WRITE_SECTORS;
    sIpcCommand.count = 1;
    // not cache aligned, use a temp buffer
    memcpy(sTempBuffers[0], (u8*)buffer, 512);
    dc_flushRange(sTempBuffers[0], 512);
    for (u32 i = 0; i < count; i++)
    {
        sIpcCommand.buffer = sTempBuffers[i & 1];
        sIpcCommand.sector = sector + i;
        sendIpcCommand();
        if (i != count - 1)
        {
            memcpy(sTempBuffers[(i + 1) & 1], (u8*)buffer + 512 * (i + 1), 512);
            dc_flushRange(sTempBuffers[(i + 1) & 1], 512);
        }
        while (!isArm7FsOperationComplete());
    }
    memcpy((u8*)buffer + 512 * (count - 1), sTempBuffers[(count - 1) & 1], 512);
}

extern "C" void fs_writeSectors(FsDevice device, const void* buffer, u32 sector, u32 count)
{
    if (count == 0)
        return;
    if (((u32)buffer >> 24) != 2 || ((u32)buffer & 0x1F))
    {
        writeSectorsNotCacheAligned(device, buffer, sector, count);
    }
    else
    {
        FsWaitToken waitToken;
        fs_writeCacheAlignedSectorsAsync(device, buffer, sector, count, &waitToken);
        fs_waitForCompletion(&waitToken, false);
    }
}
