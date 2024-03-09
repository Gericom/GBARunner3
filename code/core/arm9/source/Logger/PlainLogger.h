#pragma once
#include "mini-printf.h"
#include "ILogger.h"
#include "IOutputStream.h"
#include "Cpsr.h"

class PlainLogger : public ILogger
{
    LogLevel _maxLogLevel;
    IOutputStream* _outputStream;

    char _logBuffer[128];

public:
    PlainLogger(LogLevel maxLogLevel, IOutputStream* outputStream)
        : _maxLogLevel(maxLogLevel), _outputStream(outputStream) { }

    void LogV(LogLevel level, const char* fmt, va_list vlist) override
    {
        if (level > _maxLogLevel)
            return;
        u32 irqs = arm_disableIrqs();
        {
            mini_vsnprintf(_logBuffer, sizeof(_logBuffer), fmt, vlist);
            _outputStream->Write(_logBuffer);
        }
        arm_restoreIrqs(irqs);
    }
};