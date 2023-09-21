#include "common.h"
#include <string.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include "Emulator/IoRegisters.h"
#include "IpcChannels.h"
#include "GbaSoundIpcCommand.h"
#include "GbaSound.h"

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
gbas_shared_t gGbaSoundShared;

static void resetFifo(gbas_direct_channel_t* channel)
{
    channel->readOffset = 0;
    channel->writeOffset = 0;
    channel->curPlaySampleCount = 0;
    channel->curPlaySamples = 0;
    channel->curSample = 0;
    channel->isInitial = true;
}

static void writeFifo32(gbas_direct_channel_t* channel, u32 val, u32 mask)
{
    u32 old = channel->fifo[channel->writeOffset];
    old &= ~mask;
    old |= val & mask;
    channel->fifo[channel->writeOffset] = old;
    channel->writeOffset = (channel->writeOffset + 1) & 7;
}

void gbas_init(void)
{
    memset(&gGbaSoundShared, 0, sizeof(gGbaSoundShared));
    resetFifo(&gGbaSoundShared.directChannels[0]);
    resetFifo(&gGbaSoundShared.directChannels[1]);
    ipc_sendWordDirect(
        ((((u32)&gGbaSoundShared) >> 5) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
        (GBA_SOUND_IPC_CMD_SETUP << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_GBA_SOUND);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
    ipc_enableArm7Irq();
}

void gbas_writeSoundRegister(u32 address, u32 value, u32 length)
{
    address -= 0x04000000;
    if (address >= 0xA0 && address <= 0xA7)
    {
        u32 mask;
        if(length == 1)
            mask = 0xFF;
        else if(length == 2)
            mask = 0xFFFF;
        else
            mask = 0xFFFFFFFF;
        mask <<= (address & 3) * 8;
        value <<= (address & 3) * 8;
        if(address & 4)
        {
            writeFifo32(&gGbaSoundShared.directChannels[1], value, mask);
        }
        else
        {
            writeFifo32(&gGbaSoundShared.directChannels[0], value, mask);
        }
    }
    else if (address >= 0x60 && address < 0xB0)
    {
        for (u32 i = 0; i < length; ++i)
        {
            if (address == 0x82)
            {
                //SOUNDCNT_H
                emu_ioRegisters[address] = value;
                // gbs_setMixVolume(value & 3);

                gGbaSoundShared.directChannels[0].volume = (value >> 2) & 1;
                gGbaSoundShared.directChannels[1].volume = (value >> 3) & 1;
            }
            else if (address == 0x83)
            {
                emu_ioRegisters[address] = value & 0x77;
                gGbaSoundShared.directChannels[0].enables = value & 3;
                gGbaSoundShared.directChannels[0].timerIdx = (value >> 2) & 1;
                if (value & (1 << 3))
                {
                    resetFifo(&gGbaSoundShared.directChannels[0]);
                }
                
                gGbaSoundShared.directChannels[1].enables = (value >> 4) & 3;
                gGbaSoundShared.directChannels[1].timerIdx = (value >> 6) & 1;
                if (value & (1 << 7))
                {
                    resetFifo(&gGbaSoundShared.directChannels[1]);
                }
            }
            else
            {
                if (address == 0x84)
                {
                    gGbaSoundShared.masterEnable = value & 0x80;
                }
                emu_ioRegisters[address] = value;
                // gbs_writeReg(address & 0xFF, value & 0xFF);
            }
            ++address;
            value >>= 8;
        }
    }
}
