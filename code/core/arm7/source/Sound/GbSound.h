#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void gbs_init(void);
void gbs_writeReg(u32 reg, u8 val);

#ifdef __cplusplus
}
#endif
