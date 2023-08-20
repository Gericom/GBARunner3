#pragma once
#include "IOutputStream.h"

class NitroEmulatorOutputStream : public IOutputStream
{
public:
    NitroEmulatorOutputStream();

    void Write(const char* str) override;    
    void Flush() override { }
};