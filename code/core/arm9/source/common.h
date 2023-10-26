#pragma once
#include <nds/ndstypes.h>
#include <stddef.h>

typedef u16 bool16;

#ifdef __cplusplus

#include "Logger/ILogger.h"
#include "Core/Heap/GlobalHeap.h"

extern ILogger* gLogger;

#endif
