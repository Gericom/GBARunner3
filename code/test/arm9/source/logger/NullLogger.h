#pragma once
#include "ILogger.h"

class NullLogger : public ILogger
{
public:
    void LogV(LogLevel level, const char* fmt, va_list vlist) override { }
};