#pragma once
#ifdef __BLOCKSDS__
#define REG_SOUNDxCNT SCHANNEL_CR
#define REG_SOUNDxSAD SCHANNEL_SOURCE
#define REG_SOUNDxLEN SCHANNEL_LENGTH
#define REG_SOUNDxPNT SCHANNEL_REPEAT_POINT

#define sdmmc_sd_startup() SDMMC_init(SDMMC_DEV_CARD)
#define sdmmc_sdcard_readsectors(sector, count, buffer) SDMMC_readSectors(SDMMC_DEV_CARD, sector, buffer, count)
#define sdmmc_sdcard_writesectors(sector, count, buffer) SDMMC_writeSectors(SDMMC_DEV_CARD, sector, buffer, count)
#endif
