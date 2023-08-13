/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <nds.h>
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_FAT		0 //dldi
#define DEV_SD		1 //dsi sd
#define DEV_PC		2 //image on pc via agb semihosting

static u32 sAgbMem;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

 DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

 DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv == DEV_PC)
    {
        sAgbMem = *(u32*)0x027FFF7C;
        return 0;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

 DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    if (pdrv == DEV_PC)
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



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

 DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    if (pdrv == DEV_PC)
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


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

 DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    return RES_OK;
}

