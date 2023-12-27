#include "common.h"
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <string.h>
#include "ff.h"
#include "IpcChannels.h"
#include "cp15.h"
#include "FsIpc.h"
#include "diskio.h"

static u32 sAgbMem;

extern "C" DSTATUS disk_status(BYTE pdrv)
{
    return 0;
}

extern "C" DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv == DEV_FAT)
    {
        // already initialized by the bootstrapper
        return 0;
    }
    else if (pdrv == DEV_SD)
    {
        // already initialized on the arm7
        return 0;
    }
    else if (pdrv == DEV_PC)
    {
        sAgbMem = *(u32*)0x027FFF7C;
        return 0;
    }

    return STA_NOINIT;
}

extern "C" DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv == DEV_FAT)
    {
        fs_readSectors(FS_DEVICE_DLDI, buff, sector, count);
        return RES_OK;
    }
    else if (pdrv == DEV_SD)
    {
        fs_readSectors(FS_DEVICE_DSI_SD, buff, sector, count);
        return RES_OK;
    }
    else if (pdrv == DEV_PC)
    {
        u32 agbMem = sAgbMem;
        *(vu16*)(agbMem + 0x10002) = 1;
        *(vu32*)(agbMem + 0x10004) = sector;
        *(vu32*)(agbMem + 0x10008) = count;
        (*(vu32*)0x04000210) &= ~(1 << 13); // disable gba irq
        (*(vu32*)0x04000214) = 1 << 13; // ack gba irq
        *(vu16*)(agbMem + 0x10000) = 0x55;
        while (!((*(vu32*)0x04000214) & (1 << 13))); // wait for gba irq
        if (*(vu16*)(agbMem + 0x10000) != 0xAA)
            return RES_ERROR;
        memcpy(buff, (const void*)(agbMem + 0x10020), count * 512);
        return RES_OK;
    }

    return RES_PARERR;
}

#if FF_FS_READONLY == 0

extern "C" DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv == DEV_FAT)
    {
        fs_writeSectors(FS_DEVICE_DLDI, buff, sector, count);
        return RES_OK;
    }
    else if (pdrv == DEV_SD)
    {
        fs_writeSectors(FS_DEVICE_DSI_SD, buff, sector, count);
        return RES_OK;
    }
    else if (pdrv == DEV_PC)
    {
        u32 agbMem = *(u32*)0x027FFF7C;
        memcpy((void*)(agbMem + 0x10020), buff, count * 512);
        *(vu16*)(agbMem + 0x10002) = 2;
        *(vu32*)(agbMem + 0x10004) = sector;
        *(vu32*)(agbMem + 0x10008) = count;
        (*(vu32*)0x04000210) &= ~(1 << 13); // disable gba irq
        (*(vu32*)0x04000214) = 1 << 13; // ack gba irq
        *(vu16*)(agbMem + 0x10000) = 0x55;
        while (!((*(vu32*)0x04000214) & (1 << 13))); // wait for gba irq
        if (*(vu16*)(agbMem + 0x10000) != 0xAA)
            return RES_ERROR;
        return RES_OK;
    }

    return RES_PARERR;
}

#endif

extern "C" DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    return RES_OK;
}
