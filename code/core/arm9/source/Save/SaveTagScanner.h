#pragma once
#include "Fat/ff.h"
#include "SaveTypeInfo.h"

#define SAVE_TAG_SCANNER_TEMP_BUFFER_SIZE   (64 * 1024)

class SaveTagScanner
{
    SaveType IdentifySaveTypeFromFirst4TagBytes(u32 first4TagBytes);
    const SaveTypeInfo* GetSaveTypeInfoFromTag(SaveType saveType, const u8* tempBuffer, u32 searchBufPtr);
public:
    /// @brief Scans the given romFile for known save tags.
    /// @param romFile The rom file to scan.
    /// @param tempBuffer A temporarily buffer of SAVE_TAG_SCANNER_TEMP_BUFFER_SIZE used for scanning.
    /// @param tagRomAddress When successful this will contain the rom address of the found save tag.
    /// @return Save type information when a tag was found, or nullptr otherwise.
    const SaveTypeInfo* FindSaveTag(FIL* romFile, u8* tempBuffer, u32& tagRomAddress);
};
