#pragma once

bool sram_patchV110(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
bool sram_patchV111(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer);
