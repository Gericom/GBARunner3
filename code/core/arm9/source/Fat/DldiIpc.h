#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void dldi_readSectors(void* buffer, u32 sector, u32 count);
void dldi_writeSectors(const void* buffer, u32 sector, u32 count);

#ifdef __cplusplus
}
#endif
