#pragma once
#include "Fat/ff.h"
#include "SaveType.h"

struct SaveTypeInfo
{
    char tag[16];
    u16 tagLength;
    SaveType type;
    u32 size;
    bool (*patchFunc)(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
};
