#include "common.h"
#include <string.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include "Emulator/IoRegisters.h"
#include "VirtualMachine/VMDtcm.h"
#include "GbaSio.h"
#include "GbaSioDefinitions.h"
#include "GbaSioIpcCommand.h"
#include "GbaIoRegOffsets.h"
#include "IpcChannels.h"
#include "cp15.h"

// todo: make this configurable at runtime
#define SIO_DEVICE_RFU

[[gnu::section(".ewram.bss"), gnu::aligned(32)]]
sio_shared_t gGbaSioShared;

static bool16 sTransferBusy = false;

static inline u32 getSioMode(u32 sioCnt, u32 rcnt)
{
    u32 mode = (rcnt & GBA_RCNT_MODE_MASK) | (sioCnt & GBA_SIOCNT_MODE_MASK);
    if (mode < GBA_RCNT_MODE_GPIO)
    {
        return mode & GBA_SIOCNT_MODE_MASK;
    }
    else
    {
        return mode & GBA_RCNT_MODE_MASK;
    }
}

void sio_init(void)
{
    memset(&gGbaSioShared, 0, sizeof(gGbaSioShared));
    ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SIO,
        ((((u32)&gGbaSioShared) >> 5) << 3) |
        GBA_SIO_IPC_CMD_SETUP));
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

void sio_finishTransfer(void)
{
    if (sTransferBusy)
    {
        u32 sioCnt = *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIOCNT);
        if (!(sioCnt & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL))
        {
            *(u32*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIODATA32) = gGbaSioShared.sioToGbaData32;
        }
        else
        {
            gGbaSioShared.si = true;
        }
        sioCnt &= ~GBA_SIOCNT_ENABLE;
        if (sioCnt & GBA_SIOCNT_IRQ)
        {
            vm_emulatedIfImeIe |= 1 << 7; // SIO IRQ
        }
        *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIOCNT) = sioCnt;
        sTransferBusy = false;
    }
}

bool sio_sioCntStore16(u32 newValue)
{
    bool updateIrqs = false;
    u32 rcnt = *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_RCNT);
    switch (getSioMode(newValue, rcnt))
    {
        case GBA_SIOCNT_MODE_NORMAL_8:
        case GBA_SIOCNT_MODE_NORMAL_32:
        {
#ifdef SIO_DEVICE_RFU
            dc_drainWriteBuffer();
            dc_invalidateLine(&gGbaSioShared.sioToGbaData32);
            u32 oldValue = *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIOCNT);

            gGbaSioShared.so = (newValue & GBA_SIOCNT_NORMAL_SO) != 0;

            if (!(newValue & GBA_SIOCNT_ENABLE))
            {
                sTransferBusy = false;
            }

            if (!sTransferBusy && (newValue & GBA_SIOCNT_ENABLE))
            {
                sTransferBusy = true;
                gGbaSioShared.gbaToSioData32 = *(u32*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIODATA32);
                if (newValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL)
                {
                    *(u32*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIODATA32) = gGbaSioShared.sioToGbaData32;
                }
                dc_drainWriteBuffer();
                while (ipc_isSendFifoFull());
                ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SIO, GBA_SIO_IPC_CMD_RFU_TRANSFER | (newValue << 3)));
            }
            else if (sTransferBusy && (newValue & GBA_SIOCNT_ENABLE) && (oldValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL) != (newValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL))
            {
                gGbaSioShared.gbaToSioData32 = *(u32*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIODATA32);
                if (newValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL)
                {
                    *(u32*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIODATA32) = gGbaSioShared.sioToGbaData32;
                }
                dc_drainWriteBuffer();
                while (ipc_isSendFifoFull());
                ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SIO, GBA_SIO_IPC_CMD_RFU_TRANSFER | (newValue << 3)));
            }

            if (newValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL)
            {
                if (!(oldValue & GBA_SIOCNT_NORMAL_SO) && (newValue & GBA_SIOCNT_NORMAL_SO))
                {
                    gGbaSioShared.si = false;
                }
            }
            else
            {
                if (!(oldValue & GBA_SIOCNT_NORMAL_SO) && (newValue & GBA_SIOCNT_NORMAL_SO))
                {
                    gGbaSioShared.si = true;
                }
                else if ((oldValue & GBA_SIOCNT_NORMAL_SO) && !(newValue & GBA_SIOCNT_NORMAL_SO))
                {
                    gGbaSioShared.si = false;
                }

                if ((oldValue & GBA_SIOCNT_NORMAL_SO) != (newValue & GBA_SIOCNT_NORMAL_SO))
                {
                    dc_drainWriteBuffer();
                    while (ipc_isSendFifoFull());
                    ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SIO, GBA_SIO_IPC_CMD_SO_CHANGED));
                }
            }
#else
            newValue |= GBA_SIOCNT_SI;
            if ((newValue & GBA_SIOCNT_ENABLE) && (newValue & GBA_SIOCNT_CLOCK_SOURCE_INTERNAL))
            {
                // emulate that the transfer ends when an internal clock is used
                newValue &= ~GBA_SIOCNT_ENABLE;
                if (newValue & GBA_SIOCNT_IRQ)
                {
                    vm_emulatedIfImeIe |= 1 << 7; // SIO IRQ
                    updateIrqs = true;
                }
            }
#endif
            break;
        }
        case GBA_SIOCNT_MODE_MULTI:
        {
            newValue &= ~(GBA_SIOCNT_MULTI_ERROR | GBA_SIOCNT_MULTI_ID_MASK | GBA_SIOCNT_MULTI_SD | GBA_SIOCNT_SI);
            newValue |= GBA_SIOCNT_MULTI_SD | GBA_SIOCNT_SI;
            break;
        }
    }

    *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIOCNT) = newValue;
    return updateIrqs;
}

void sio_rcntStore16(u32 newValue)
{
    u32 oldValue = *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_RCNT);
    *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_RCNT) = newValue;
    u32 sioCnt = *(u16*)((u32)emu_ioRegisters + GBA_REG_OFFS_SIOCNT);
    switch (getSioMode(sioCnt, newValue))
    {
        case GBA_RCNT_MODE_GPIO:
        {
#ifdef SIO_DEVICE_RFU
            if (!(oldValue & GBA_RCNT_SD_VALUE) && (newValue & GBA_RCNT_SD_OUTPUT))
            {
                ipc_sendWordDirect(IPC_FIFO_MSG(IPC_CHANNEL_GBA_SIO, GBA_SIO_IPC_CMD_RFU_RESET));
            }
#endif
            break;
        }
    }
}
