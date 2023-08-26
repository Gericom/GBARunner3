.section ".ewram.bss", "aw"

#include "SdCacheDefs.h"

.balign SDC_BLOCK_SIZE

.global sdc_cache
sdc_cache:
    .space SDC_SIZE

.end
