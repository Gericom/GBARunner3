#pragma once
#include "mini-printf.h"
#include "ILogger.h"
#include "IOutputStream.h"

class PlainLogger : public ILogger
{
    LogLevel _maxLogLevel;
    IOutputStream* _outputStream;

    // char _logBuffer[512];

public:
    PlainLogger(LogLevel maxLogLevel, IOutputStream* outputStream)
        : _maxLogLevel(maxLogLevel), _outputStream(outputStream) { }

    void LogV(LogLevel level, const char* fmt, va_list vlist) override
    {
        if (level > _maxLogLevel)
            return;
        char logBuffer[128];
        mini_vsnprintf(logBuffer, sizeof(logBuffer), fmt, vlist);
        _outputStream->Write(logBuffer);
    }
};