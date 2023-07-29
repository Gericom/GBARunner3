#pragma once
#include "IOutputStream.h"

class NullOutputStream : public IOutputStream
{
public:
    void Write(const char* str) { }
    void Flush() { }
};