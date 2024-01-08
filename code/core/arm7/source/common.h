#pragma once
#include <nds.h>

#ifdef __cplusplus

#include "Logger/ILogger.h"

extern ILogger* gLogger;

#else

void logFromC(const char* fmt, ...);

#endif
