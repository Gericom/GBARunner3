#include "common.h"
#include "wifi.h"
#include "wifiRf.h"

static bool waitRfReady()
{
    for (int i = 0; i < 10240; i++)
    {
        if (!REG_WIFI_RFR_STATUS)
        {
            return true;
        }
    }
    return false;
}

void wifi_writeRf(u32 val)
{
    REG_WIFI_RFR_DAT = val & 0xFFFF;
    REG_WIFI_RFR_CMD = val >> 16;
    waitRfReady();
}