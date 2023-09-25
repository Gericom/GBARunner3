#include "common.h"
#include <string.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include "Emulator/IoRegisters.h"
#include "IpcChannels.h"
#include "GbaIoRegOffsets.h"
#include "GbaSoundDefinitions.h"
#include "GbaSoundIpcCommand.h"
#include "GbaSound.h"

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
gbas_shared_t gGbaSoundShared;

static const u8 sRegisterMasks[] =
{
    (u8)~0x80, (u8)~0xFF, (u8)~0x3F, (u8)~0x00, (u8)~0xFF, (u8)~0xBF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x3F, (u8)~0x00, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xBF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x1F, (u8)~0xFF, (u8)~0xFF, (u8)~0x1F, (u8)~0xFF, (u8)~0xBF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0xFF, (u8)~0x00, (u8)~0xFF, (u8)~0xFF, (u8)~0x00, (u8)~0xBF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x88, (u8)~0x00, (u8)~0x00, (u8)~0x88, (u8)~0x7F, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x00, (u8)~0x00, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00,
    (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00
};

static void resetFifo(gbas_direct_channel_t* channel)
{
    channel->readOffset = 0;
    channel->writeOffset = 0;
    channel->curPlaySampleCount = 0;
    channel->curPlaySamples = 0;
}

static void writeFifo32(gbas_direct_channel_t* channel, u32 val, u32 mask)
{
    u32 old = channel->fifo[channel->writeOffset];
    old &= ~mask;
    old |= val & mask;
    channel->fifo[channel->writeOffset] = old;
    channel->writeOffset = (channel->writeOffset + 1) & 7;
}

static void writeGbAudioReg(u32 reg, u32 val)
{
    if (!(gGbaSoundShared.soundCntX & GBA_SOUNDCNT_X_MASTER_ENABLE) && reg != GBA_REG_OFFS_SOUNDCNT_X && reg < 0x90)
        return;

    emu_ioRegisters[reg] = val & sRegisterMasks[reg - GBA_REG_OFFS_SOUND1CNT_L];

    if (reg == GBA_REG_OFFS_SOUNDCNT_X)
    {
        if (!(val & GBA_SOUNDCNT_X_MASTER_ENABLE))
        {
            emu_ioRegisters[reg] = 0;
            gGbaSoundShared.soundCntX = 0;
        }
        else
        {
            emu_ioRegisters[reg] |= GBA_SOUNDCNT_X_MASTER_ENABLE;
            gGbaSoundShared.soundCntX |= GBA_SOUNDCNT_X_MASTER_ENABLE;
        }
    }
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
    if (address >= GBA_REG_OFFS_FIFO_A && address <= (GBA_REG_OFFS_FIFO_B + 3))
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
        if (address & 4)
        {
            writeFifo32(&gGbaSoundShared.directChannels[1], value, mask);
        }
        else
        {
            writeFifo32(&gGbaSoundShared.directChannels[0], value, mask);
        }
    }
    else if (address >= GBA_REG_OFFS_SOUND1CNT_L && address < GBA_REG_OFFS_DMA0SAD)
    {
        if (length == 1)
        {
            // send to the arm 7
            while (ipc_isSendFifoFull());
            ipc_sendWordDirect(
                ((value & 0xFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4 + 8)) |
                ((address & 0xFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
                (GBA_SOUND_IPC_CMD_GB_REG_WRITE_8 << IPC_FIFO_MSG_CHANNEL_BITS) |
                IPC_CHANNEL_GBA_SOUND);
        }
        else if (length == 2)
        {
            // send to the arm 7
            while (ipc_isSendFifoFull());
            ipc_sendWordDirect(
                ((value & 0xFFFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4 + 8)) |
                ((address & 0xFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
                (GBA_SOUND_IPC_CMD_GB_REG_WRITE_16 << IPC_FIFO_MSG_CHANNEL_BITS) |
                IPC_CHANNEL_GBA_SOUND);
        }
        else
        {
            // send to the arm 7
            while (ipc_isSendFifoFull());
            ipc_sendWordDirect(
                ((value & 0xFFFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4 + 8)) |
                ((address & 0xFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
                (GBA_SOUND_IPC_CMD_GB_REG_WRITE_16 << IPC_FIFO_MSG_CHANNEL_BITS) |
                IPC_CHANNEL_GBA_SOUND);
            while (ipc_isSendFifoFull());
            ipc_sendWordDirect(
                ((value >> 16) << (IPC_FIFO_MSG_CHANNEL_BITS + 4 + 8)) |
                (((address + 2) & 0xFF) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
                (GBA_SOUND_IPC_CMD_GB_REG_WRITE_16 << IPC_FIFO_MSG_CHANNEL_BITS) |
                IPC_CHANNEL_GBA_SOUND);
        }

        for (u32 i = 0; i < length; ++i)
        {
            if (address == GBA_REG_OFFS_SOUNDCNT_H)
            {
                emu_ioRegisters[address] = value;
                ((u8*)&gGbaSoundShared.soundCntH)[0] = value;
                writeGbAudioReg(address & 0xFF, value & 0xFF);
            }
            else if (address == (GBA_REG_OFFS_SOUNDCNT_H + 1))
            {
                emu_ioRegisters[address] = value & 0x77;
                ((u8*)&gGbaSoundShared.soundCntH)[1] = value & 0x77;

                if (value & (GBA_SOUNDCNT_H_DIRECT_A_RESET >> 8))
                {
                    resetFifo(&gGbaSoundShared.directChannels[0]);
                }
                
                if (value & (GBA_SOUNDCNT_H_DIRECT_B_RESET >> 8))
                {
                    resetFifo(&gGbaSoundShared.directChannels[1]);
                }
            }
            else if (address >= GBA_REG_OFFS_SOUNDBIAS && address <= (GBA_REG_OFFS_SOUNDBIAS + 3))
            {
                emu_ioRegisters[address] = value;
            }
            else
            {
                writeGbAudioReg(address & 0xFF, value & 0xFF);
            }
            ++address;
            value >>= 8;
        }
    }
}
