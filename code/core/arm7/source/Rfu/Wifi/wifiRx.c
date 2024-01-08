#include "common.h"
#include "wifi.h"
#include "wifiRx.h"

static wifi_rx_done_callback_t sRxDoneCallback;
static void* sRxDoneCallbackArg;

void wifi_irqRxStart()
{
    REG_WIFI_ISR = WIFI_IRQ_RX_START;
    // logFromC("Rx start!\n");
}

void wifi_irqRxEnd()
{
    REG_WIFI_ISR = WIFI_IRQ_RX_END;
    if (sRxDoneCallback)
    {
        sRxDoneCallback(sRxDoneCallbackArg);
    }
    else
    {
        REG_WIFI_RXBUF_BNR = REG_WIFI_RXBUF_CUR;
    }
}

void wifi_initRx()
{
    //setup rx buffer
    REG_WIFI_MDP_CONFIG = WIFI_MDP_CONFIG_RX_BUF_ENABLE;
    REG_WIFI_RXBUF_STR = 0x4000 + WIFI_RAM_RX_BUF_OFFSET;
    REG_WIFI_RXBUF_WCUR = WIFI_RAM_RX_BUF_OFFSET >> 1;
    REG_WIFI_RXBUF_END = 0x5F60;
    REG_WIFI_RXBUF_BNR = WIFI_RAM_RX_BUF_OFFSET >> 1;
    REG_WIFI_RDMA_JUMP = 0x5F5E;
    REG_WIFI_MDP_CONFIG = WIFI_MDP_CONFIG_RX_BUF_ENABLE | WIFI_MDP_CONFIG_SET_RX_BUF_EMPTY;
    REG_WIFI_WDP_LAST_RX_ADR.address16[1] = 0xFFFF;
    REG_WIFI_DCF_LAST_ADRS.address16[0] = 0xFFFF;
    REG_WIFI_DCF_LAST_ADRS.address16[1] = 0xFFFF;
    REG_WIFI_MP_LAST_SEQCTRL = 0xFFFF;
    WIFI_RAM->unk5F60[11] = 0xFFFF;
}

void wifi_setRxDoneCallback(wifi_rx_done_callback_t callback, void* arg)
{
    sRxDoneCallback = callback;
    sRxDoneCallbackArg = arg;
}
