#include <nds.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/spi/spi.h>
#include "wifi.h"
#include "nvram.h"
#include "wifiFlash.h"

void wifi_initFlashData()
{
    u32 oldIrq = rtos_disableIrqs();
    {
        //Read command
        spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, NVRAM_CMD_READ);

        //Address
        spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, (0 >> 16) & 0xFF);
        spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, (0 >> 8) & 0xFF);
        spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, (0) & 0xFF);

        u16* dst = (u16*)&WIFI_RAM->firmData;
        //Read 2 bytes a time, because wifi ram does not support byte writes
        for(int i = 0; i < 512; i += 2) 
        {            
            u8 a = spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, 0);
            u8 b = spi_transferByte(SPICNT_DEVICE_FIRMWARE | SPICNT_SPEED_4_MHZ, 0);
            *dst++ = a | (b << 8);
        }

        REG_SPICNT = 0;
    }
    rtos_restoreIrqs(oldIrq);
}