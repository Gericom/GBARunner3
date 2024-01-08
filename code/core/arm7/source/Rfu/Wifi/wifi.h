#pragma once
#include "wifiRegisters.h"
#include "nvram.h"

typedef struct
{
    bool use2Mbps;
    u32 bkReg;
    u16 bbpCnt;
} wifi_work_t;

extern wifi_work_t gWifiWork;

typedef struct
{
    struct
    {
        u16 version : 2;
        u16 type : 2;
        u16 subType : 4;
        u16 toDS : 1;
        u16 fromDS : 1;
        u16 moreFrag : 1;
        u16 retry : 1;
        u16 powerMgmt : 1;
        u16 moreData : 1;
        u16 wep : 1;
        u16 order : 1;
    } frameCtrl;
    u16 durationId;
    wifi_macaddr_t addr1;
    wifi_macaddr_t addr2;
    wifi_macaddr_t addr3;
    struct
    {
        u16 seqNum : 12;
        u16 fragNum : 4;
    } seqCtrl;
} wifi_pkt_ieee_header_t;

#define WIFI_RAM_SIZE				0x2000
//self chosen values:
#define WIFI_RAM_TX_BUF_LENGTH		0x800
#define WIFI_RAM_RX_BUF_LENGTH		0x1560

#define WIFI_RAM_TX_BUF_OFFSET		(0x200)
#define WIFI_RAM_RX_BUF_OFFSET		(WIFI_RAM_TX_BUF_OFFSET + WIFI_RAM_TX_BUF_LENGTH)

typedef struct
{
    nvram_data_t firmData;
    volatile u8 txBuf[WIFI_RAM_TX_BUF_LENGTH];
    volatile u8 rxBuf[WIFI_RAM_RX_BUF_LENGTH];
    u16 unk5F60[16];
    u16 wepKeys[4][16];
} wifi_ram_t;

static_assert(sizeof(wifi_ram_t) == WIFI_RAM_SIZE, "Invalid wifi ram length!");

#define WIFI_RAM					((wifi_ram_t*)0x4804000)

#define WIFI_SHUTDOWN_POWERON		0
#define WIFI_SHUTDOWN_POWEROFF		1

#define WIFI_BUFFERING_SELECT_BSSID	(1 << 10)

typedef void(*wifi_mp_done_callback_t)(void* arg);

void wifi_setMpDoneCallback(wifi_mp_done_callback_t callback, void* arg);
bool wifi_setMode(u8 mode);
bool wifi_setWepMode(u8 mode);
void wifi_setMacAddress(const wifi_macaddr_t* address);
void wifi_setBssid(const wifi_macaddr_t* bssid);
void wifi_setRetryLimit(u8 limit);
bool wifi_setBeaconInterval(u16 interval);
bool wifi_setDtimInterval(u8 interval);

void wifi_updateTxTimeStampOffset();
void wifi_setUse2Mbps(bool use2Mbps);
void wifi_setShortPreamble(bool shortPreamble);
bool wifi_setChannel(int channel);
bool wifi_setActiveZoneTime(u16 time);

void wifi_init();
void wifi_deinit();
void wifi_stop();
void wifi_start();
