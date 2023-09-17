#pragma once
#include "SaveTypeInfo.h"

bool eeprom_patchV111(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
bool eeprom_patchV120(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
bool eeprom_patchV124(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
bool eeprom_patchV126(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
