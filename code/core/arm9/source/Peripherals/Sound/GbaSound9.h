#pragma once
#include "GbaSound.h"

extern gbas_shared_t gGbaSoundShared;

#ifdef __cplusplus
extern "C" {
#endif

void gbas_init(void);
void gbas_writeSoundRegister(u32 address, u32 value, u32 length);

#ifdef __cplusplus
}
#endif
