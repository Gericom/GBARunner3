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
    if (!gGbaSoundShared.masterEnable && reg != GBA_REG_OFFS_SOUNDCNT_X)
        return;

    switch (reg)
    {
        case GBA_REG_OFFS_SOUND1CNT_L: //NR10
            emu_ioRegisters[reg] = val & ~0x80;
            break;
        case GBA_REG_OFFS_SOUND1CNT_H: //NR11
            emu_ioRegisters[reg] = val & ~0x3F;
            break;
        case (GBA_REG_OFFS_SOUND1CNT_H + 1): //NR12
            emu_ioRegisters[reg] = val;
            break;
        case GBA_REG_OFFS_SOUND1CNT_X: //NR13
            break;
        case (GBA_REG_OFFS_SOUND1CNT_X + 1): //NR14
            emu_ioRegisters[reg] = val & ~0xBF;
            break;
        case GBA_REG_OFFS_SOUND2CNT_L: //NR21
            emu_ioRegisters[reg] = val & ~0x3F;
            break;
        case (GBA_REG_OFFS_SOUND2CNT_L + 1): //NR22
            emu_ioRegisters[reg] = val;
            break;
        case GBA_REG_OFFS_SOUND2CNT_H: //NR23
            break;
        case (GBA_REG_OFFS_SOUND2CNT_H + 1): //NR24
            emu_ioRegisters[reg] = val & ~0xBF;
            break;
        case GBA_REG_OFFS_SOUND3CNT_L: //NR30
            emu_ioRegisters[reg] = val & ~0x1F;
            break;
        case GBA_REG_OFFS_SOUND3CNT_H: //NR31
            break;
        case (GBA_REG_OFFS_SOUND3CNT_H + 1): //NR32
            emu_ioRegisters[reg] = val & ~0x1F;
            break;
        case GBA_REG_OFFS_SOUND3CNT_X: //NR33
            break;
        case (GBA_REG_OFFS_SOUND3CNT_X + 1): //NR34
            emu_ioRegisters[reg] = val & ~0xBF;
            break;
        case GBA_REG_OFFS_SOUND4CNT_L: //NR41
            break;
        case (GBA_REG_OFFS_SOUND4CNT_L + 1): //NR42
            emu_ioRegisters[reg] = val;
            break;
        case GBA_REG_OFFS_SOUND4CNT_H: //NR43
            emu_ioRegisters[reg] = val;
            break;
        case (GBA_REG_OFFS_SOUND4CNT_H + 1): //NR44
            emu_ioRegisters[reg] = val & ~0xBF;
            break;
        case GBA_REG_OFFS_SOUNDCNT_L: //NR50
            emu_ioRegisters[reg] = val & 0x77;
            break;
        case (GBA_REG_OFFS_SOUNDCNT_L + 1): //NR51
            emu_ioRegisters[reg] = val;
            break;

        case GBA_REG_OFFS_SOUNDCNT_X: //NR52
        {
            if (!(val & GBA_SOUNDCNT_X_MASTER_ENABLE))
            {
                gGbaSoundShared.masterEnable = false;
                emu_ioRegisters[reg] &= ~0x80;
                gGbaSoundShared.soundCntX &= ~0x80;
            }
            else
            {
                gGbaSoundShared.masterEnable = true;
                emu_ioRegisters[reg] |= 0x80;
                gGbaSoundShared.soundCntX |= 0x80;
            }
            break;
        }
        
        case GBA_REG_OFFS_WAVE_RAM0_L:
        case (GBA_REG_OFFS_WAVE_RAM0_L + 1):
        case GBA_REG_OFFS_WAVE_RAM0_H:
        case (GBA_REG_OFFS_WAVE_RAM0_H + 1):
        case GBA_REG_OFFS_WAVE_RAM1_L:
        case (GBA_REG_OFFS_WAVE_RAM1_L + 1):
        case GBA_REG_OFFS_WAVE_RAM1_H:
        case (GBA_REG_OFFS_WAVE_RAM1_H + 1):
        case GBA_REG_OFFS_WAVE_RAM2_L:
        case (GBA_REG_OFFS_WAVE_RAM2_L + 1):
        case GBA_REG_OFFS_WAVE_RAM2_H:
        case (GBA_REG_OFFS_WAVE_RAM2_H + 1):
        case GBA_REG_OFFS_WAVE_RAM3_L:
        case (GBA_REG_OFFS_WAVE_RAM3_L + 1):
        case GBA_REG_OFFS_WAVE_RAM3_H:
        case (GBA_REG_OFFS_WAVE_RAM3_H + 1):
            emu_ioRegisters[reg] = val;
            break;
    }

    // send to the arm 7
    while (ipc_isSendFifoFull());
    ipc_sendWordDirect(
        ((val) << (IPC_FIFO_MSG_CHANNEL_BITS + 4 + 8)) |
        ((reg) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
        (GBA_SOUND_IPC_CMD_GB_REG_WRITE << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_GBA_SOUND);
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
