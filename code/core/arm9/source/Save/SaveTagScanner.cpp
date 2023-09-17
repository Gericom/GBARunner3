#include "common.h"
#include <algorithm>
#include <array>
#include "SdCache/SdCache.h"
#include "SaveEeprom.h"
#include "SaveFlash.h"
#include "SaveSram.h"
#include "SaveTagScanner.h"

#define SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE  (SAVE_TAG_SCANNER_TEMP_BUFFER_SIZE >> 1)

#define TAG_START_FLAS  0x53414C46
#define TAG_START_SRAM  0x4D415253
#define TAG_START_EEPR  0x52504545

static constexpr auto sSaveTypeInfos = std::to_array<const SaveTypeInfo>
({
    {"EEPROM_V111", 12, SAVE_TYPE_EEPROM_V111, 512, eeprom_patchV111},
    {"EEPROM_V120", 12, SAVE_TYPE_EEPROM_V120, 8 * 1024, eeprom_patchV120},
    {"EEPROM_V121", 12, SAVE_TYPE_EEPROM_V121, 8 * 1024, eeprom_patchV120},
    {"EEPROM_V122", 12, SAVE_TYPE_EEPROM_V122, 8 * 1024, eeprom_patchV120},
    {"EEPROM_V124", 12, SAVE_TYPE_EEPROM_V124, 8 * 1024, eeprom_patchV124},
    {"EEPROM_V125", 12, SAVE_TYPE_EEPROM_V125, 8 * 1024, eeprom_patchV124},
    {"EEPROM_V126", 12, SAVE_TYPE_EEPROM_V126, 8 * 1024, eeprom_patchV126},

    {"FLASH_V120", 11, SAVE_TYPE_FLASH_V120, 64 * 1024, flash_patchV120},
    {"FLASH_V121", 11, SAVE_TYPE_FLASH_V121, 64 * 1024, flash_patchV120},
    {"FLASH_V123", 11, SAVE_TYPE_FLASH_V123, 64 * 1024, flash_patchV123},
    {"FLASH_V124", 11, SAVE_TYPE_FLASH_V124, 64 * 1024, flash_patchV123},
    {"FLASH_V125", 11, SAVE_TYPE_FLASH_V125, 64 * 1024, flash_patchV123},
    {"FLASH_V126", 11, SAVE_TYPE_FLASH_V126, 64 * 1024, flash_patchV126},
    {"FLASH512_V130", 14, SAVE_TYPE_FLASH512_V130, 64 * 1024, flash_patch512V130},
    {"FLASH512_V131", 14, SAVE_TYPE_FLASH512_V131, 64 * 1024, flash_patch512V130},
    {"FLASH512_V133", 14, SAVE_TYPE_FLASH512_V133, 64 * 1024, flash_patch512V130},
    {"FLASH1M_V102", 13, SAVE_TYPE_FLASH1M_V102, 128 * 1024, flash_patch1MV102},
    {"FLASH1M_V103", 13, SAVE_TYPE_FLASH1M_V103, 128 * 1024, flash_patch1MV103 },

    //Fast SRAM does not require patching
    {"SRAM_F_V100", 12, SAVE_TYPE_SRAM_F_V100, 32 * 1024, nullptr},
    {"SRAM_F_V102", 12, SAVE_TYPE_SRAM_F_V102, 32 * 1024, nullptr},
    {"SRAM_F_V103", 12, SAVE_TYPE_SRAM_F_V103, 32 * 1024, nullptr},

    {"SRAM_V110", 10, SAVE_TYPE_SRAM_V110, 32 * 1024, sram_patchV110},
    {"SRAM_V111", 10, SAVE_TYPE_SRAM_V111, 32 * 1024, sram_patchV111},
    {"SRAM_V112", 10, SAVE_TYPE_SRAM_V112, 32 * 1024, sram_patchV111},
    {"SRAM_V113", 10, SAVE_TYPE_SRAM_V113, 32 * 1024, sram_patchV111},
});

const SaveTypeInfo* SaveTagScanner::FindSaveTag(FIL* romFile, u8* tempBuffer, u32& tagRomAddress)
{
    tagRomAddress = 0;
    f_rewind(romFile);
    UINT read;
    u32 curAddr = 0;
    if (f_read(romFile, tempBuffer, SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE, &read) != FR_OK)
    {
        return nullptr;
    }
    int searchBufPtr = 0;
    while (curAddr < f_size(romFile))
    {
        if (searchBufPtr == 0)
        {
            if (f_read(romFile, tempBuffer + SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE,
                SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE, &read) != FR_OK)
            {
                return nullptr;
            }
        }
        else if (searchBufPtr == SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE)
        {
            if (f_read(romFile, tempBuffer, SAVE_TAG_SCANNER_TEMP_BUFFER_HALF_SIZE, &read) != FR_OK)
            {
                return nullptr;
            }
        }

        SaveType saveType = IdentifySaveTypeFromFirst4TagBytes(*(u32*)&tempBuffer[searchBufPtr]);
        if (saveType != SAVE_TYPE_NONE)
        {
            auto saveTypeInfo = GetSaveTypeInfoFromTag(saveType, tempBuffer, searchBufPtr);
            if (saveTypeInfo)
            {
                tagRomAddress = curAddr;
                return saveTypeInfo;
            }
        }
        searchBufPtr = (searchBufPtr + 4) & (SAVE_TAG_SCANNER_TEMP_BUFFER_SIZE - 1);
        curAddr += 4;
    }
    return nullptr;
}

SaveType SaveTagScanner::IdentifySaveTypeFromFirst4TagBytes(u32 first4TagBytes)
{
    switch (first4TagBytes)
    {
        case TAG_START_FLAS:
        {
            return SAVE_TYPE_FLASH;
        }
        case TAG_START_SRAM:
        {
            return SAVE_TYPE_SRAM;
        }
        case TAG_START_EEPR:
        {
            return SAVE_TYPE_EEPROM;
        }
        default:
        {
            return SAVE_TYPE_NONE;
        }
    }
}

const SaveTypeInfo* SaveTagScanner::GetSaveTypeInfoFromTag(SaveType saveType, const u8* tempBuffer, u32 searchBufPtr)
{
    for (const auto& saveTypeInfo : sSaveTypeInfos)
    {
        if ((saveTypeInfo.type & SAVE_TYPE_MASK) != saveType)
        {
            continue;
        }
        bool found = true;
        u32 alignedTagLength = (saveTypeInfo.tagLength + 3) & ~3;
        for (u32 j = 4; j < alignedTagLength; j += 4)
        {
            const u32 expected = *(u32*)&saveTypeInfo.tag[j];
            const u32 actual = *(u32*)&tempBuffer[(searchBufPtr + j) & (SAVE_TAG_SCANNER_TEMP_BUFFER_SIZE - 1)];
            if (actual != expected)
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return &saveTypeInfo;
        }
    }
    return nullptr;
}
