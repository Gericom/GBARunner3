#pragma once
#include <stdarg.h>

enum class LogLevel
{
    Off,

    Fatal,
    Error,
    Warning,
    Info,
    Debug,
    Trace,

    All
};

class ILogger
{
public:
    virtual ~ILogger() { }

    void Log(LogLevel level, const char* fmt, ...)
    {
        va_list vlist;
        va_start(vlist, fmt);
        LogV(level, fmt, vlist);
        va_end(vlist);
    }

    virtual void LogV(LogLevel level, const char* fmt, va_list vlist) = 0;
};