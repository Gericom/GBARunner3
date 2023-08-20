#pragma once

class IOutputStream
{
public:
    virtual ~IOutputStream() { }

    virtual void Write(const char* str) = 0;
    virtual void Flush() = 0;
};