# DSi (TWL) SDMMC code

## Usage
* Call `TMIO_init()` once **after** irqInit() (libnds).
* Call `SDMMC_init(const u8 devNum)`.

If SDMMC_init() succeeded you can use all other SDMMC_...() functions. SDMMC_init() can be called again at any time to reinitialize the SD card/eMMC provided you call SDMMC_deinit() first.

DMA is supported for SDMMC_readSectors() and SDMMC_writeSectors() by passing a NULL pointer for buf and setting up a NDMA channel prior to calling these functions. Note that the TMIO driver does not support unaligned access for DMA and buffers not a multiple of 4 bytes (for CPU and DMA transfers). It's recommended to add a bounce buffer or cache to make sure buffers are 4 bytes aligned.

## License
This software is licensed under the MIT license. See LICENSE.txt for details.

Exemptions:
* The [BlocksDS](https://github.com/blocksds) project and its members are exempt from MIT licensing. They are permitted to relicense the source code of this repository, at their discretion, to the [zlib license](https://zlib.net/zlib_license.html). In doing so they may alter, supplement or entirely remove the copyright notice for each file they choose to relicense.
* The [dsi-modchip](https://github.com/dsi-modchip) project and its members are exempt from MIT licensing. They are permitted to relicense the source code of this repository, at their discretion, to the [LGPL license version 3 or later](https://www.gnu.org/licenses/lgpl-3.0.html). In doing so they may alter, supplement or entirely remove the copyright notice for each file they choose to relicense.