#pragma once
#include <memory>
#include "mini-printf.h"
#include "ILogger.h"
#include "IOutputStream.h"

class PlainLogger : public ILogger
{
    LogLevel _maxLogLevel;
    std::unique_ptr<IOutputStream> _outputStream;

    char _logBuffer[512];

public:
    PlainLogger(LogLevel maxLogLevel, std::unique_ptr<IOutputStream> outputStream)
        : _maxLogLevel(maxLogLevel), _outputStream(std::move(outputStream))
    { }

    void LogV(LogLevel level, const char* fmt, va_list vlist) override
    {
        if (level > _maxLogLevel)
            return;
        mini_vsnprintf(_logBuffer, sizeof(_logBuffer), fmt, vlist);
        _outputStream->Write(_logBuffer);
    }
};