#pragma once

#define WIFI_ASSOCIATION_RESPONSE_STATUS_SUCCESS    0
#define WIFI_ASSOCIATION_RESPONSE_STATUS_FULL       0x11 // cannot connect more childs

#ifdef __cplusplus
extern "C" {
#endif

void rfuw_stopSendParentBeacons(void);
void rfuw_startParentMode(u16 parentId, u8 entrySlot, const u8* userName, const u8* gameName);
void rfuw_stopParentMode(void);
void rfuw_sendAssociationResponseToChild(const wifi_macaddr_t* childMacAddress, u16 associationId, u16 status);
void rfuw_startSearchMode(void);
void rfuw_stopSearchMode(void);
void rfuw_startConnectToParent(const wifi_macaddr_t* parentBssid);
void rfuw_setMultiPollChildReplyData(const wifi_macaddr_t* parentBssid, const u32* childData, u32 childDataLength);
void rfuw_sendMultiPollParentCmd(const u32* parentData, u32 parentDataLength, u32 childMask, u32 childCount);

#ifdef __cplusplus
}
#endif
