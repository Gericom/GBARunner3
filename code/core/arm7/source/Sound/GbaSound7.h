#pragma once
#include "GbaSound.h"

extern gbas_shared_t* gSoundSharedData;

#ifdef __cplusplus
extern "C" {
#endif

void gbas_init(gbas_shared_t* sharedData);
void gbas_setTimerReload(u32 timer, u16 reload);
void gbas_setTimerControl(u32 timer, u16 control);
void gbas_updateMixer(s16* outLeft, s16* outRight);

#ifdef __cplusplus
}
#endif
