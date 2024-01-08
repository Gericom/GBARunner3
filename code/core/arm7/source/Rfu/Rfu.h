#pragma once
#include "GbaSio.h"
#include "Wifi/wifiCommon.h"

typedef struct
{
    u16 parentId;
    u8 entrySlot;
    u8 padding;
    u8 gameName[16];
    u8 userName[8];
} rfu_beacon_data_t;

#ifdef __cplusplus
extern "C" {
#endif

void rfu_init(sio_shared_t* sharedData);
void rfu_reset(void);
void rfu_doTransfer(bool internalClock);
void rfu_soChanged(void);
void rfu_notifyBeaconReceived(const rfu_beacon_data_t* beaconData, const wifi_macaddr_t* parentBssid);
void rfu_notifyAssociationRequestReceived(const wifi_macaddr_t* childMacAddress);
void rfu_notifyAssociationResponseReceived(u16 associationId, u16 status);
void rfu_notifyParentDataReceived(const u8* data, u32 dataLength);
void rfu_notifyChildDataReceived(u32 childId, const u8* data, u32 dataLength);
void rfu_notifyMpEnd(u32 failChildMask);
void rfu_frame_update(void);

#ifdef __cplusplus
}
#endif
