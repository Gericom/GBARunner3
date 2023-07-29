#pragma once
#include "IOutputStream.h"

#define REG_NOCASH_STRING_OUT   (*(vu32*)0x04FFFA10)
#define REG_NOCASH_CHAR_OUT     (*(vu32*)0x04FFFA1C)

class NocashOutputStream : public IOutputStream
{
public:
    void Write(const char* str) override
    {
        REG_NOCASH_STRING_OUT = (u32)str;
    }

    void Flush() override { }
};