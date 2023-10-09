#include "common.h"
#include <libtwl/spi/spiPmic.h>
#include "SystemIpcService.h"

void SystemIpcService::HandleMessage(u32 data)
{
    switch (data & 0xF)
    {
        case SYSTEM_IPC_CMD_SET_TOP_BACKLIGHT:
        {
            pmic_setTopBacklightEnable((data >> 4) & 1);
            SendResponseMessage(0);
            break;
        }
        case SYSTEM_IPC_CMD_SET_BOTTOM_BACKLIGHT:
        {
            pmic_setBottomBacklightEnable((data >> 4) & 1);
            SendResponseMessage(0);
            break;
        }
    }
}
