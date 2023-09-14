#pragma once

#include <nds/disc_io.h>

extern u8 _dldi_start[16 * 1024];
extern FN_MEDIUM_STARTUP _DLDI_startup_ptr;
extern FN_MEDIUM_READSECTORS _DLDI_readSectors_ptr;
extern FN_MEDIUM_WRITESECTORS _DLDI_writeSectors_ptr;
