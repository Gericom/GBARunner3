#pragma once

#define SDC_SIZE                (1 * 1024 * 1024)
#define SDC_BLOCK_SIZE          (2 * 1024)
#define SDC_BLOCK_MASK          (SDC_BLOCK_SIZE - 1)
#define SDC_BLOCK_SHIFT         11
#define SDC_BLOCK_COUNT         (SDC_SIZE / SDC_BLOCK_SIZE)
#define SDC_BLOCK_INVALID       0xFFFF
#define SDC_ROM_BLOCK_INVALID   0xFFFF

#define SDC_ROM_BLOCK_COUNT     (32 * 1024 * 1024 / SDC_BLOCK_SIZE)
