#include "common.h"
#include "Save.h"
#include "SaveCopy.h"
#include "SaveTypeInfo.h"
#include "SaveEeprom.h"

//todo: Moero!! Jaleco Collection (Japan) reports EEPROM_V124, but the signatures below don't work!

static const u32 sReadEepromDwordV111Sig[] = { 0xB0AAB5B0u, 0x6079466Fu, 0x80081C39u, 0x88011C38u };
static const u32 sReadEepromDwordV120Sig[] = { 0xB0A2B570u, 0x04001C0Du, 0x48030C03u, 0x88806800u };
static const u32 sProgramEepromDwordV111Sig[] = { 0xB0AAB580u, 0x6079466Fu, 0x80081C39u, 0x88011C38u };

//changed in EEPROM_V124
static const u32 sProgramEepromDwordV120Sig[] = { 0xB0A9B530u, 0x04001C0Du, 0x48030C04u, 0x88806800u };

//changed in EEPROM_V126
static const u32 sProgramEepromDwordV124Sig[] = { 0xB0ACB5F0u, 0x04001C0Du, 0x06120C01u, 0x48030E17u };

static const u32 sProgramEepromDwordV126Sig[] = { 0x4647B5F0u, 0xB0ACB480u, 0x04001C0Eu, 0x06120C05u };

static u16 readEepromDword(u16 epAdr, u16* dst)
{
    const u8* save = &gSaveData[epAdr << 3];
    for (int i = 0; i < 8; ++i)
    {
        sav_writeGbaByte(&((u8*)dst)[7 - i], *save++);
    }
    return 0;
}

static u16 programEepromDword(u16 epAdr, const u16* src)
{
    u8* save = &gSaveData[epAdr << 3];
    for (int i = 0; i < 8; ++i)
    {
        *save++ = sav_readGbaByte(&((const u8*)src)[7 - i]);
    }
    return 0;
}

bool eeprom_patchV111(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadEepromDwordV111Sig, 0, (void*)readEepromDword)
        && sav_tryPatchFunction(sProgramEepromDwordV111Sig, 1, (void*)programEepromDword);
}

bool eeprom_patchV120(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadEepromDwordV120Sig, 0, (void*)readEepromDword)
        && sav_tryPatchFunction(sProgramEepromDwordV120Sig, 1, (void*)programEepromDword);
}

bool eeprom_patchV124(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadEepromDwordV120Sig, 0, (void*)readEepromDword)
        && sav_tryPatchFunction(sProgramEepromDwordV124Sig, 1, (void*)programEepromDword);
}

bool eeprom_patchV126(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadEepromDwordV120Sig, 0, (void*)readEepromDword)
        && sav_tryPatchFunction(sProgramEepromDwordV126Sig, 1, (void*)programEepromDword);
}
