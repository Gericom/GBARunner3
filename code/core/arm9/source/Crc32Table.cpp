#include "common.h"
#include "Crc32Table.h"

[[gnu::section(".ewram")]]
const Crc32Table gCrc32Table { };
