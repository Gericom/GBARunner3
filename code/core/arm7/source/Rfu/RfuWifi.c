#include "common.h"
#include <string.h>
#include <libtwl/dma/dmaNitro.h>
#include "Wifi/wifi.h"
#include "Wifi/wifiTx.h"
#include "Wifi/wifiRx.h"
#include "Rfu.h"
#include "RfuWifi.h"

#define BEACON_TYPE_MULTICARD   1
#define GAME_ID_GBARUNNER3      0x00409BA3

#pragma pack(1)
typedef struct
{
    wifi_pkt_ieee_header_t ieeeHeader;
    u64 timestamp;
    u16 beaconInterval;
    u16 capabilityInfo;
} wifi_beacon_header_t;

typedef struct
{
    u8 tag;
    u8 length;
    u8 nintendoBeaconOui[4];
    u16 steppingOffset;
    u16 lcdVideoSync;
    u32 fixedId;
    u32 gameId;
    u16 streamCode;
    u8 payloadLength;
    u8 beaconType;
    u16 cmdDataSize;
    u16 replyDataSize;
} wifi_beacon_nintendo_tag_t;

typedef struct
{
    wifi_beacon_header_t header;
    struct
    {
        u8 tag;
        u8 length;
        u8 rates[2];
    } supportedRates;
    struct
    {
        u8 tag;
        u8 length;
        u8 channel;
    } distributionChannel;
    struct
    {
        u8 tag;
        u8 length;
        u8 data[5];
    } timVector;
    wifi_beacon_nintendo_tag_t nintendoTag;
    rfu_beacon_data_t nintendoTagPayload;
} rfu_beacon_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    rfu_beacon_t beacon;
    u32 checksum;
} rfu_beacon_tx_t;

typedef struct
{
    wifi_pkt_ieee_header_t ieeeHeader;
    u16 capabilityInfo;
    u16 listenInterval;
} wifi_association_request_header_t;

typedef struct
{
    wifi_association_request_header_t header;
    struct
    {
        u8 tag;
        u8 length;
        u8 ssid[32];
    } ssid;
    struct
    {
        u8 tag;
        u8 length;
        u8 rates[2];
    } supportedRates;
} wifi_association_request_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    wifi_association_request_t associationRequest;
    u32 checksum;
} wifi_association_request_tx_t;

typedef struct
{
    wifi_pkt_ieee_header_t ieeeHeader;
    u16 capabilityInfo;
    u16 status;
    u16 associationId;
} wifi_association_response_header_t;

typedef struct
{
    wifi_association_response_header_t header;
    struct
    {
        u8 tag;
        u8 length;
        u8 rates[2];
    } supportedRates;
} wifi_association_response_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    wifi_association_response_t associationResponse;
    u32 checksum;
} wifi_association_response_tx_t;

typedef struct
{
    wifi_pkt_ieee_header_t ieeeHeader;
    u16 childTime;
    u16 childMask;
} wifi_mp_parent_packet_header_t;

typedef struct
{
    wifi_mp_parent_packet_header_t header;
    u8 payload[87];
    u8 payloadLength;
} wifi_mp_parent_packet_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    wifi_mp_parent_packet_t mpParentPacket;
    u32 checksum;
} wifi_mp_parent_packet_tx_t;

typedef struct
{
    wifi_pkt_ieee_header_t ieeeHeader;
    u8 payload[16];
    u8 payloadLength;
    u8 childId;
} wifi_mp_child_packet_t;

typedef struct
{
    wifi_pkt_hw_header_tx_t hwHeader;
    wifi_mp_child_packet_t mpChildPacket;
    u32 checksum;
} wifi_mp_child_packet_tx_t;

#pragma pack()

#define TX_RAM_OFFSET_BEACON                    0x000
#define TX_RAM_OFFSET_ASSOCIATION_REQUEST       0x100
#define TX_RAM_OFFSET_ASSOCIATION_RESPONSE      0x200
#define TX_RAM_OFFSET_MP_PARENT                 0x300
#define TX_RAM_OFFSET_MP_CHILD_1                0x400
#define TX_RAM_OFFSET_MP_CHILD_2                0x500

static rfu_beacon_tx_t sBeaconFrame =
{
    .hwHeader = { 0, 0, 0, 0, 0x14, sizeof(rfu_beacon_t) + 4 },
    .beacon =
    {
        .header =
        {
            .ieeeHeader =
            {
                .frameCtrl = { 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
                .durationId = 0,
                .addr1 = { .address = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } }, // broadcast
                .addr2 = { .address = { 0, 0, 0, 0, 0, 0 } }, // src MAC; should be set to MAC
                .addr3 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID; should be set to MAC
                .seqCtrl = { 0, 0 }
            },
            .timestamp = 0,
            .beaconInterval = 0,
            .capabilityInfo = 0x21
        },
        .supportedRates = { 1, 2, { 0x82, 0x84 } },
        .distributionChannel = { 3, 1, 13 },
        .timVector = { 5, 5, { 0, 2, 0, 0, 0 } },
        .nintendoTag =
        {
            0xDD,
            sizeof(wifi_beacon_nintendo_tag_t) - 2 + sizeof(rfu_beacon_data_t),
            { 0x00, 0x09, 0xBF, 0x00 },
            0xA,
            0,
            0x00000100,
            GAME_ID_GBARUNNER3,
            0,
            sizeof(rfu_beacon_data_t),
            BEACON_TYPE_MULTICARD,
            sizeof(wifi_mp_parent_packet_t) + 4,
            sizeof(wifi_mp_child_packet_t) + 4
        },
        .nintendoTagPayload = { 0 }
    },
    .checksum = 0
};

static wifi_association_request_tx_t sAssociationRequestFrame =
{
    .hwHeader = { 0, 0, 0, 0, 0x14, sizeof(wifi_association_request_t) + 4 },
    .associationRequest =
    {
        .header =
        {
            .ieeeHeader =
            {
                .frameCtrl = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                .durationId = 0,
                .addr1 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID
                .addr2 = { .address = { 0, 0, 0, 0, 0, 0 } }, // src MAC; should be set to MAC
                .addr3 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID
                .seqCtrl = { 0, 0 }
            },
            .capabilityInfo = 0x21,
            .listenInterval = 1
        },
        .ssid =
        {
            0, 32,
            {
                GAME_ID_GBARUNNER3 & 0xFF,
                (GAME_ID_GBARUNNER3 >> 8) & 0xFF,
                (GAME_ID_GBARUNNER3 >> 16) & 0xFF,
                (GAME_ID_GBARUNNER3 >> 24) & 0xFF,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            }
        },
        .supportedRates = { 1, 2, { 0x82, 0x84 } }
    },
    .checksum = 0
};

static wifi_association_response_tx_t sAssociationResponseFrame =
{
    .hwHeader = { 0, 0, 0, 0, 0x14, sizeof(wifi_association_response_t) + 4 },
    .associationResponse =
    {
        .header =
        {
            .ieeeHeader =
            {
                .frameCtrl = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
                .durationId = 0,
                .addr1 = { .address = { 0, 0, 0, 0, 0, 0 } }, // child MAC
                .addr2 = { .address = { 0, 0, 0, 0, 0, 0 } }, // src MAC; should be set to MAC
                .addr3 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID
                .seqCtrl = { 0, 0 }
            },
            .capabilityInfo = 0x21,
            .status = 0,
            .associationId = 0
        },
        .supportedRates = { 1, 2, { 0x82, 0x84 } }
    },
    .checksum = 0
};

static wifi_mp_parent_packet_tx_t sParentFrame =
{
    .hwHeader = { 0, 0, 0, 0, 0x14, sizeof(wifi_mp_parent_packet_t) + 4 },
    .mpParentPacket =
    {
        .header =
        {
            .ieeeHeader =
            {
                .frameCtrl = { 0, 2, 2, 0, 1, 0, 0, 0, 0, 0, 0 },
                .durationId = 0,
                .addr1 = { .address = { 0x03, 0x09, 0xBF, 0x00, 0x00, 0x00 } },
                .addr2 = { .address = { 0, 0, 0, 0, 0, 0 } }, // src MAC; should be set to MAC
                .addr3 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID
                .seqCtrl = { 0xFFF, 0xF }
            },
            .childTime = 0,
            .childMask = 0
        }
    },
    .checksum = 0
};

static wifi_mp_child_packet_tx_t sChildReplyFrame =
{
    .hwHeader = { 0, 0, 0, 0, 0x14, sizeof(wifi_mp_child_packet_t) + 4 },
    .mpChildPacket =
    {
        .ieeeHeader =
        {
            .frameCtrl = { 0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
            .durationId = 0,
            .addr1 = { .address = { 0, 0, 0, 0, 0, 0 } }, // BSSID
            .addr2 = { .address = { 0, 0, 0, 0, 0, 0 } }, // src MAC; should be set to MAC
            .addr3 = { .address = { 0x03, 0x09, 0xBF, 0x00, 0x00, 0x10 } },
            .seqCtrl = { 0xFFF, 0xF }
        }
    },
    .checksum = 0
};

static u32 wrapRxPtr(u32 ptr)
{
    u32 start = REG_WIFI_RXBUF_STR & 0x1FFE;
    u32 end = REG_WIFI_RXBUF_END & 0x1FFE;
    if (ptr >= end)
        ptr -= end - start;
    return ptr;
}

static u16 rxRamRead(u32 offset)
{
    return *(u16*)(((u32)WIFI_RAM) + wrapRxPtr(offset));
}

static u8 rxRamReadByte(u32 offset)
{
    return *(u8*)(((u32)WIFI_RAM) + wrapRxPtr(offset));
}

static void parentModeRxDoneCallback(void* arg)
{
    while (REG_WIFI_RXBUF_CUR != REG_WIFI_RXBUF_BNR)
    {
        int base = REG_WIFI_RXBUF_BNR << 1;
        int hdrLen = rxRamRead(base + 8);
        int len = ((hdrLen + 3) & ~3) + 12;

        u32 frameType = rxRamRead(base) & 0xF;
        if (frameType == 0xE)
        {
            u8 data[18];
            for (u32 i = 0; i < sizeof(data); i += 2)
            {
                *(u16*)&data[i] = rxRamRead(base + 12 + sizeof(wifi_pkt_ieee_header_t) + i);
            }
            rfu_notifyChildDataReceived(data[17], data, data[16]);
        }
        else if (frameType == 0 && rxRamRead(base + 12) == 0 &&
            rxRamRead(base + 12 + offsetof(wifi_association_request_t, ssid) + 2) == (GAME_ID_GBARUNNER3 & 0xFFFF) &&
            rxRamRead(base + 12 + offsetof(wifi_association_request_t, ssid) + 4) == (GAME_ID_GBARUNNER3 >> 16))
        {
            wifi_macaddr_t childMacAddress;
            childMacAddress.address16[0] = rxRamRead(base + 12 + 10);
            childMacAddress.address16[1] = rxRamRead(base + 12 + 12);
            childMacAddress.address16[2] = rxRamRead(base + 12 + 14);
            rfu_notifyAssociationRequestReceived(&childMacAddress);
        }

        REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;
    }
}

static void childModeRxDoneCallback(void* arg)
{
    while (REG_WIFI_RXBUF_CUR != REG_WIFI_RXBUF_BNR)
    {
        u32 base = REG_WIFI_RXBUF_BNR << 1;
        u32 hdrLen = rxRamRead(base + 8);
        u32 len = ((hdrLen + 3) & ~3) + 12;

        u32 frameType = rxRamRead(base) & 0xF;
        if (frameType == 1)
        {
            u32 tagPtr = base + 12 + sizeof(wifi_beacon_header_t);
            while (tagPtr < base + 12 + hdrLen)
            {
                u32 tag = rxRamReadByte(tagPtr);
                u32 tagLength = rxRamReadByte(tagPtr + 1);
                if (tag == 0xDD && tagLength >= sizeof(wifi_beacon_nintendo_tag_t))
                {
                    // Nintendo beacon
                    u32 gameIdLo = rxRamRead(tagPtr + offsetof(wifi_beacon_nintendo_tag_t, gameId));
                    u32 gameIdHi = rxRamRead(tagPtr + offsetof(wifi_beacon_nintendo_tag_t, gameId) + 2);
                    u32 gameId = (gameIdHi << 16) | gameIdLo;
                    if (gameId == GAME_ID_GBARUNNER3)
                    {
                        // GBARunner3 beacon
                        rfu_beacon_data_t beaconData;
                        for (u32 i = 0; i < sizeof(rfu_beacon_data_t); i += 2)
                        {
                            *(u16*)((u32)&beaconData + i) = rxRamRead(tagPtr + sizeof(wifi_beacon_nintendo_tag_t) + i);
                        }
                        wifi_macaddr_t bssid;
                        bssid.address16[0] = rxRamRead(base + 12 + 16);
                        bssid.address16[1] = rxRamRead(base + 12 + 18);
                        bssid.address16[2] = rxRamRead(base + 12 + 20);
                        rfu_notifyBeaconReceived(&beaconData, &bssid);
                    }
                }
                tagPtr += tagLength + 2;
            }
        }

        REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;
    }
}

static void receiveParentDataRxDoneCallback(void* arg)
{
    while (REG_WIFI_RXBUF_CUR != REG_WIFI_RXBUF_BNR)
    {
        u32 base = REG_WIFI_RXBUF_BNR << 1;
        u32 hdrLen = rxRamRead(base + 8);
        u32 len = ((hdrLen + 3) & ~3) + 12;

        u32 frameType = rxRamRead(base) & 0xF;
        if (frameType == 0xC)
        {
            u8 data[88];
            for (u32 i = 0; i < sizeof(data); i += 2)
            {
                *(u16*)&data[i] = rxRamRead(base + 12 + sizeof(wifi_mp_parent_packet_header_t) + i);
            }
            rfu_notifyParentDataReceived(data, data[87]);
        }

        REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;
    }
}

static void mpDoneCallback(void* arg)
{
    u16 childMask = *(u16*)&WIFI_RAM->txBuf[TX_RAM_OFFSET_MP_PARENT + 2];
    rfu_notifyMpEnd((childMask >> 1) & 0xF);
}

static void connectToParentRxDoneCallback(void* arg)
{
    while (REG_WIFI_RXBUF_CUR != REG_WIFI_RXBUF_BNR)
    {
        u32 base = REG_WIFI_RXBUF_BNR << 1;
        u32 hdrLen = rxRamRead(base + 8);
        u32 len = ((hdrLen + 3) & ~3) + 12;

        u32 frameType = rxRamRead(base) & 0xF;
        if (frameType == 0 && rxRamRead(base + 12) == 0x10)
        {
            u32 status = rxRamRead(base + 12 + offsetof(wifi_association_response_header_t, status));
            u32 associationId = rxRamRead(base + 12 + offsetof(wifi_association_response_header_t, associationId));
            if (status == WIFI_ASSOCIATION_RESPONSE_STATUS_SUCCESS)
            {
                REG_WIFI_TXQ_RESET = WIFI_TXQ_RESET_KEYIN | WIFI_TXQ_RESET_KEYOUT;
                REG_WIFI_KSID = (associationId & 3) + 1;
                wifi_setRxDoneCallback(receiveParentDataRxDoneCallback, NULL);
            }
            else
            {
                // nack, stop receiving
                // REG_WIFI_MDP_CONFIG = 0;
                wifi_setRxDoneCallback(NULL, NULL);
            }
            rfu_notifyAssociationResponseReceived(associationId, status);
        }

        REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;
    }
}

void rfuw_stopSendParentBeacons(void)
{
    // stop wifi hardware sending beacons
    REG_WIFI_BEACON_ADRS = 0;
}

void rfuw_startParentMode(u16 parentId, u8 entrySlot, const u8* userName, const u8* gameName)
{
    // start sending beacons with wifi hardware to announce the available parent
    REG_WIFI_BEACON_ADRS = 0;

    REG_WIFI_RX_CNTOVF_INT_MASK = 0x1FFF;
    REG_WIFI_BUFFERING_SELECT = 0x301;
    REG_WIFI_DS_MASK = WIFI_DS_MASK_IGNORE_STA_TO_STA | WIFI_DS_MASK_IGNORE_DS_TO_STA | WIFI_DS_MASK_IGNORE_DS_TO_DS;
    REG_WIFI_TX_CONFIG = 0xE000;
    REG_WIFI_TSF_ENABLE = 1;
    REG_WIFI_TBTT_ENABLE = 1;
    REG_WIFI_TXQ_OPEN = WIFI_TXQ_MP;

    sBeaconFrame.beacon.nintendoTagPayload.parentId = parentId;
    sBeaconFrame.beacon.nintendoTagPayload.entrySlot = entrySlot;
    sBeaconFrame.beacon.nintendoTagPayload.padding = 0;
    memcpy(sBeaconFrame.beacon.nintendoTagPayload.userName, userName, 8);
    memcpy(sBeaconFrame.beacon.nintendoTagPayload.gameName, gameName, 16);
    sBeaconFrame.beacon.header.ieeeHeader.addr2 = WIFI_RAM->firmData.wifiData.macAddress;
    sBeaconFrame.beacon.header.ieeeHeader.addr3 = WIFI_RAM->firmData.wifiData.macAddress;
    wifi_setBssid(&WIFI_RAM->firmData.wifiData.macAddress);
    REG_WIFI_BCN_PARAM_OFFSET = 0x15;
    wifi_setBeaconInterval(206);
    wifi_setDtimInterval(2);
    wifi_setActiveZoneTime(0xA);
    dmaCopyWords(3, &sBeaconFrame, (void*)&WIFI_RAM->txBuf[TX_RAM_OFFSET_BEACON], sizeof(sBeaconFrame));
    REG_WIFI_BEACON_ADRS = 0x8000 | ((WIFI_RAM_TX_BUF_OFFSET + TX_RAM_OFFSET_BEACON) >> 1);

    wifi_setRxDoneCallback(parentModeRxDoneCallback, NULL);
    wifi_setMpDoneCallback(mpDoneCallback, NULL);
    REG_WIFI_ISR = WIFI_IRQ_MP_END;
    REG_WIFI_IMR |= WIFI_IRQ_MP_END;
}

void rfuw_stopParentMode(void)
{
    REG_WIFI_IMR &= ~WIFI_IRQ_MP_END;
    REG_WIFI_ISR = WIFI_IRQ_MP_END;
    wifi_setRxDoneCallback(NULL, NULL);
    wifi_setMpDoneCallback(NULL, NULL);
    REG_WIFI_BEACON_ADRS = 0;
}

void rfuw_sendAssociationResponseToChild(const wifi_macaddr_t* childMacAddress, u16 associationId, u16 status)
{
    sAssociationResponseFrame.associationResponse.header.ieeeHeader.addr1 = *childMacAddress;
    sAssociationResponseFrame.associationResponse.header.ieeeHeader.addr2 = WIFI_RAM->firmData.wifiData.macAddress;
    sAssociationResponseFrame.associationResponse.header.ieeeHeader.addr3 = WIFI_RAM->firmData.wifiData.macAddress;
    sAssociationResponseFrame.associationResponse.header.status = status;
    sAssociationResponseFrame.associationResponse.header.associationId = associationId;
    while (REG_WIFI_TXQ_CURR & WIFI_TXQ_TXQ1);
    dmaCopyWords(3,
        &sAssociationResponseFrame,
        (void*)&WIFI_RAM->txBuf[TX_RAM_OFFSET_ASSOCIATION_RESPONSE],
        sizeof(sAssociationResponseFrame));
    wifi_setRetryLimit(7);
    REG_WIFI_TXQ1_ADR = 0x8000 | ((WIFI_RAM_TX_BUF_OFFSET + TX_RAM_OFFSET_ASSOCIATION_RESPONSE) >> 1);
    REG_WIFI_TXQ_OPEN = WIFI_TXQ_TXQ1;
}

void rfuw_startSearchMode(void)
{
    REG_WIFI_BSSID.address16[0] = 0;
    REG_WIFI_BSSID.address16[1] = 0;
    REG_WIFI_BSSID.address16[2] = 0;
    REG_WIFI_KSID = 0;
    REG_WIFI_BUFFERING_SELECT = 0x581;
    REG_WIFI_DS_MASK = WIFI_DS_MASK_IGNORE_STA_TO_STA | WIFI_DS_MASK_IGNORE_STA_TO_DS | WIFI_DS_MASK_IGNORE_DS_TO_DS;
    wifi_setRxDoneCallback(childModeRxDoneCallback, NULL);
}

void rfuw_stopSearchMode(void)
{
    wifi_setRxDoneCallback(NULL, NULL);
}

void rfuw_startConnectToParent(const wifi_macaddr_t* parentBssid)
{
    REG_WIFI_BUFFERING_SELECT = 0x581;
    REG_WIFI_DS_MASK = WIFI_DS_MASK_IGNORE_STA_TO_STA | WIFI_DS_MASK_IGNORE_STA_TO_DS | WIFI_DS_MASK_IGNORE_DS_TO_DS;
    wifi_setBssid(parentBssid);

    REG_WIFI_ISR = WIFI_IRQ_RX_END;
    REG_WIFI_IMR |= WIFI_IRQ_RX_END;
    wifi_setRxDoneCallback(connectToParentRxDoneCallback, NULL);

    sAssociationRequestFrame.associationRequest.header.ieeeHeader.addr1 = *parentBssid;
    sAssociationRequestFrame.associationRequest.header.ieeeHeader.addr2 = WIFI_RAM->firmData.wifiData.macAddress;
    sAssociationRequestFrame.associationRequest.header.ieeeHeader.addr3 = *parentBssid;
    while (REG_WIFI_TXQ_CURR & WIFI_TXQ_TXQ1);
    dmaCopyWords(3, &sAssociationRequestFrame, (void*)&WIFI_RAM->txBuf[TX_RAM_OFFSET_ASSOCIATION_REQUEST], sizeof(sAssociationRequestFrame));
    wifi_setRetryLimit(7);
    REG_WIFI_TXQ1_ADR = 0x8000 | ((WIFI_RAM_TX_BUF_OFFSET + TX_RAM_OFFSET_ASSOCIATION_REQUEST) >> 1);
    REG_WIFI_TXQ_OPEN = WIFI_TXQ_TXQ1;
}

void rfuw_setMultiPollChildReplyData(const wifi_macaddr_t* parentBssid, const u32* childData, u32 childDataLength)
{
    u32 oldBuffer = ((REG_WIFI_KEYOUT_ADR & 0xFFF) << 1) - WIFI_RAM_TX_BUF_OFFSET;
    u32 newBuffer = oldBuffer == TX_RAM_OFFSET_MP_CHILD_1 ? TX_RAM_OFFSET_MP_CHILD_2 : TX_RAM_OFFSET_MP_CHILD_1;
    sChildReplyFrame.mpChildPacket.ieeeHeader.addr1 = *parentBssid;
    sChildReplyFrame.mpChildPacket.ieeeHeader.addr2 = WIFI_RAM->firmData.wifiData.macAddress;
    memcpy(sChildReplyFrame.mpChildPacket.payload, childData, childDataLength);
    sChildReplyFrame.mpChildPacket.payloadLength = childDataLength;
    sChildReplyFrame.mpChildPacket.childId = REG_WIFI_KSID - 1;
    dmaCopyWords(3, &sChildReplyFrame, (void*)&WIFI_RAM->txBuf[newBuffer], sizeof(sChildReplyFrame));
    wifi_setRetryLimit(7);
    REG_WIFI_KEYIN_ADR = 0x8000 | ((WIFI_RAM_TX_BUF_OFFSET + newBuffer) >> 1);
}

void rfuw_sendMultiPollParentCmd(const u32* parentData, u32 parentDataLength, u32 childMask, u32 childCount)
{
    int parentBytes = sizeof(wifi_mp_parent_packet_t) + 4;
    int childBytes = sizeof(wifi_mp_child_packet_t) + 4;
    int parentTime = parentBytes * 4 + 0x60;
    int childTime = childBytes * 4 + 0xD2;
    int allChildTime = 0xF0 + (childTime + 10) * childCount;
    sParentFrame.mpParentPacket.header.ieeeHeader.addr2 = WIFI_RAM->firmData.wifiData.macAddress;
    sParentFrame.mpParentPacket.header.ieeeHeader.addr3 = WIFI_RAM->firmData.wifiData.macAddress;
    sParentFrame.hwHeader.status2 = childMask;
    sParentFrame.mpParentPacket.header.ieeeHeader.durationId = allChildTime;
    sParentFrame.mpParentPacket.header.childTime = childTime;
    sParentFrame.mpParentPacket.header.childMask = childMask;
    memcpy(sParentFrame.mpParentPacket.payload, parentData, parentDataLength);
    sParentFrame.mpParentPacket.payloadLength = parentDataLength;
    while (REG_WIFI_TXQ_CURR & WIFI_TXQ_MP);
    dmaCopyWords(3, &sParentFrame, (void*)&WIFI_RAM->txBuf[TX_RAM_OFFSET_MP_PARENT], sizeof(sParentFrame));
    wifi_setRetryLimit(7);
    REG_WIFI_INITIAL_MP_DURATION = allChildTime;
    REG_WIFI_KEYSHARING_TXOP = childTime;
    REG_WIFI_TX_CONFIG = 0xE000;
    REG_WIFI_TMPTT_TIMER = (0x388 + (childCount * childTime) + parentTime + 0x32) / 10;
    REG_WIFI_MP_ADR = 0x8000 | ((WIFI_RAM_TX_BUF_OFFSET + TX_RAM_OFFSET_MP_PARENT) >> 1);
}
