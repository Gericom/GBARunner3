#pragma once
#ifdef LIBTWL_ARM7
#include <libtwl/rtos/rtosIrq.h>
#endif
#include "mini-printf.h"
#include "ILogger.h"
#include "IOutputStream.h"
#ifdef LIBTWL_ARM9
#include "Cpsr.h"
#endif

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
#ifdef LIBTWL_ARM7
        u32 irqs = rtos_disableIrqs();
#endif
#ifdef LIBTWL_ARM9
        u32 irqs = arm_disableIrqs();
#endif
        {
            mini_vsnprintf(_logBuffer, sizeof(_logBuffer), fmt, vlist);
            _outputStream->Write(_logBuffer);
        }
#ifdef LIBTWL_ARM7
        rtos_restoreIrqs(irqs);
#endif
#ifdef LIBTWL_ARM9
        arm_restoreIrqs(irqs);
#endif
    }
};