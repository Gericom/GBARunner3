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
    (u8)~0x88, (u8)~0x00, (u8)~0xF0, (u8)~0x88, (u8)~0x7F, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x00, (u8)~0x00, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF, (u8)~0xFF,
    (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00,
    (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00, (u8)~0x00
};

static void resetFifo(gbas_direct_channel_t* channel)
{
    channel->readOffset = 0;
    channel->writeOffset = 0;
}

static void writeFifo32(gbas_direct_channel_t* channel, u32 val, u32 mask)
{
    u32 writeOffset = channel->writeOffset;
    u32 old = channel->fifo[writeOffset];
    old &= ~mask;
    old |= val & mask;
    channel->fifo[writeOffset] = old;
    channel->writeOffset = (writeOffset + 1) & 7;
}

static void writeGbAudioReg(u32 reg, u32 val)
{
    if (!gGbaSoundShared.masterEnable && reg != GBA_REG_OFFS_SOUNDCNT_X && reg < 0x90)
        return;

    if (reg == GBA_REG_OFFS_SOUNDCNT_X)
    {
        if (val & GBA_SOUNDCNT_X_MASTER_ENABLE)
        {
            emu_ioRegisters[reg] = GBA_SOUNDCNT_X_MASTER_ENABLE;
            gGbaSoundShared.masterEnable = true;
        }
        else
        {
            emu_ioRegisters[reg] = 0;
            gGbaSoundShared.masterEnable = false;
        }
    }
    else
    {
        emu_ioRegisters[reg] = val & sRegisterMasks[reg - GBA_REG_OFFS_SOUND1CNT_L];
    }
}

void gbas_init(void)
{
    memset(&gGbaSoundShared, 0, sizeof(gGbaSoundShared));
    resetFifo(&gGbaSoundShared.directChannels[0]);
    resetFifo(&gGbaSoundShared.directChannels[1]);
    ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SOUND,
        ((((u32)&gGbaSoundShared) >> 5) << 3) |
        GBA_SOUND_IPC_CMD_SETUP));
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

void gbas_writeSoundRegister(u32 address, u32 value, u32 length)
{
    address -= 0x04000000;
    if (address >= GBA_REG_OFFS_FIFO_A && address <= (GBA_REG_OFFS_FIFO_B + 3))
    {
        gbas_direct_channel_t* channel;
        if (address & 4)
        {
            channel = &gGbaSoundShared.directChannels[1];
        }
        else
        {
            channel = &gGbaSoundShared.directChannels[0];
        }
        if (__builtin_expect(length == 4, true))
        {
            writeFifo32(channel, value, 0xFFFFFFFF);
        }
        else
        {
            u32 mask;
            if (length == 1)
                mask = 0xFF;
            else
                mask = 0xFFFF;
            mask <<= (address & 3) * 8;
            value <<= (address & 3) * 8;
            writeFifo32(channel, value, mask);
        }
    }
    else if (address >= GBA_REG_OFFS_SOUND1CNT_L && address < GBA_REG_OFFS_DMA0SAD)
    {
        // send to the arm 7
        while (ipc_isSendFifoFull());
        if (length == 1)
        {
            ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SOUND,
                ((value & 0xFF) << (3 + 8)) |
                ((address & 0xFF) << 3) |
                GBA_SOUND_IPC_CMD_GB_REG_WRITE_8));
        }
        else if (length == 2)
        {
            ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SOUND,
                ((value & 0xFFFF) << (3 + 8)) |
                ((address & 0xFF) << 3) |
                GBA_SOUND_IPC_CMD_GB_REG_WRITE_16));
        }
        else
        {
            ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SOUND,
                ((value & 0xFFFF) << (3 + 8)) |
                ((address & 0xFF) << 3) |
                GBA_SOUND_IPC_CMD_GB_REG_WRITE_16));
            while (ipc_isSendFifoFull());
            ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SOUND,
                ((value >> 16) << (3 + 8)) |
                (((address + 2) & 0xFF) << 3) |
                GBA_SOUND_IPC_CMD_GB_REG_WRITE_16));
        }

        for (u32 i = 0; i < length; ++i)
        {
            if (address == GBA_REG_OFFS_SOUNDCNT_H)
            {
                emu_ioRegisters[address] = value;
            }
            else if (address == (GBA_REG_OFFS_SOUNDCNT_H + 1))
            {
                emu_ioRegisters[address] = value & 0x77;

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
