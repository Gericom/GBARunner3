#include "common.h"
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include <string.h>
#include "ipcChannels.h"
#include "dldiIpcCommand.h"
#include "cp15.h"
#include "dldiIpc.h"

[[gnu::section(".ewram.bss")]]
alignas(32) static dldi_ipc_cmd_t sIpcCommand;
[[gnu::section(".ewram.bss")]]
alignas(32) static u8 sTempBuffers[2][512];

static u32 readSectorsCacheAligned(void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = DLDI_IPC_CMD_READ_SECTORS;
    sIpcCommand.buffer = buffer;
    sIpcCommand.sector = sector;
    sIpcCommand.count = count;
    dc_invalidateRange(buffer, 512 * count);
    dc_flushRange(&sIpcCommand, sizeof(sIpcCommand));
    ipc_sendWordDirect(((((u32)&sIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_DLDI);
    while (ipc_isRecvFifoEmpty());
    return ipc_recvWordDirect();
}

static void readSectorsNotCacheAligned(void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = DLDI_IPC_CMD_READ_SECTORS;
    sIpcCommand.count = 1;
    // not cache aligned, use a temp buffer
    dc_invalidateRange(sTempBuffers[0], 512);
    for (u32 i = 0; i < count; i++)
    {
        sIpcCommand.buffer = sTempBuffers[i & 1];
        sIpcCommand.sector = sector + i;
        dc_flushRange(&sIpcCommand, sizeof(sIpcCommand));
        ipc_sendWordDirect(((((u32)&sIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_DLDI);
        if (i != 0)
            memcpy((u8*)buffer + 512 * (i - 1), sTempBuffers[(i - 1) & 1], 512);
        if (i != count - 1)
            dc_invalidateRange(sTempBuffers[(i + 1) & 1], 512);
        while (ipc_isRecvFifoEmpty());
        ipc_recvWordDirect();
    }
    memcpy((u8*)buffer + 512 * (count - 1), sTempBuffers[(count - 1) & 1], 512);
}

extern "C" void dldi_readSectors(void* buffer, u32 sector, u32 count)
{
    if (count == 0)
        return;
    if (((u32)buffer >> 24) != 2 || ((u32)buffer & 0x1F))
        readSectorsNotCacheAligned(buffer, sector, count);
    else
        readSectorsCacheAligned(buffer, sector, count);
}

static void writeSectorsCacheAligned(const void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = DLDI_IPC_CMD_WRITE_SECTORS;
    sIpcCommand.buffer = (void*)buffer;
    sIpcCommand.sector = sector;
    sIpcCommand.count = count;
    dc_flushRange(buffer, 512 * count);
    dc_flushRange(&sIpcCommand, sizeof(sIpcCommand));
    ipc_sendWordDirect(((((u32)&sIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_DLDI);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

static void writeSectorsNotCacheAligned(const void* buffer, u32 sector, u32 count)
{
    sIpcCommand.cmd = DLDI_IPC_CMD_WRITE_SECTORS;
    sIpcCommand.count = 1;
    // not cache aligned, use a temp buffer
    memcpy(sTempBuffers[0], (u8*)buffer, 512);
    dc_flushRange(sTempBuffers[0], 512);
    for (u32 i = 0; i < count; i++)
    {
        sIpcCommand.buffer = sTempBuffers[i & 1];
        sIpcCommand.sector = sector + i;
        dc_flushRange(&sIpcCommand, sizeof(sIpcCommand));
        ipc_sendWordDirect(((((u32)&sIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_DLDI);
        if (i != count - 1)
        {
            memcpy(sTempBuffers[(i + 1) & 1], (u8*)buffer + 512 * (i + 1), 512);
            dc_flushRange(sTempBuffers[(i + 1) & 1], 512);
        }
        while (ipc_isRecvFifoEmpty());
        ipc_recvWordDirect();
    }
    memcpy((u8*)buffer + 512 * (count - 1), sTempBuffers[(count - 1) & 1], 512);
}

extern "C" void dldi_writeSectors(const void* buffer, u32 sector, u32 count)
{
    if (count == 0)
        return;
    if (((u32)buffer >> 24) != 2 || ((u32)buffer & 0x1F))
        writeSectorsNotCacheAligned(buffer, sector, count);
    else
        writeSectorsCacheAligned(buffer, sector, count);
}
