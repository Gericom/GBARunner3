#pragma once

#define SDC_SIZE                (1 * 1024 * 1024)
#define SDC_BLOCK_SHIFT         12
#define SDC_BLOCK_SIZE          (1 << SDC_BLOCK_SHIFT)
#define SDC_BLOCK_MASK          (SDC_BLOCK_SIZE - 1)
#define SDC_BLOCK_COUNT         (SDC_SIZE / SDC_BLOCK_SIZE)
#define SDC_BLOCK_INVALID       0xFFFF
#define SDC_ROM_BLOCK_INVALID   0xFFFF

#define SDC_ROM_BLOCK_COUNT     (32 * 1024 * 1024 / SDC_BLOCK_SIZE)
