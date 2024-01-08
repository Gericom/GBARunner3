#pragma once
#include "wifi.h"

#define WIFI_PKT_HW_HEADER_TX_STATUS_ERROR			0
#define WIFI_PKT_HW_HEADER_TX_STATUS_OK				1

#define WIFI_PKT_HW_HEADER_TX_SERVICE_RATE_1MBPS	10
#define WIFI_PKT_HW_HEADER_TX_SERVICE_RATE_2MBPS	20

typedef struct
{
    u16 status;
    u16 status2;
    u16 rsvRetryCount;
    u16 rsvAppRate;
    u16 serviceRate;
    u16 mpdu;
} wifi_pkt_hw_header_tx_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    wifi_pkt_ieee_header_t ieeeHeader;
    //u8 payload[0];
    //u32 fcs;
} wifi_pkt_tx_t;

#define WIFI_TX_TEST_FRAME_FC_DUR_OFF	(1 << 0)
#define WIFI_TX_TEST_FRAME_CRC_OFF		(1 << 1)
#define WIFI_TX_TEST_FRAME_SEQ_OFF		(1 << 2)


void wifi_irqTxStart();
void wifi_irqTxEnd();
void wifi_initTx();

typedef void (*wifi_tx_done_callback_t)(void* arg);

void wifi_setTxDoneCallback(wifi_tx_done_callback_t callback, void* arg);