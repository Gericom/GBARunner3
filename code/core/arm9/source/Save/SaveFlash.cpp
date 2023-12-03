#include "common.h"
#include <string.h>
#include <libtwl/mem/memSwap.h>
#include "MemFastSearch.h"
#include "Save.h"
#include "SaveSwi.h"
#include "SaveFlashDefinitions.h"
#include "SaveTypeInfo.h"
#include "MemoryEmulator/RomDefs.h"
#include "SaveFlash.h"

#define MAKER_ID_MACRONIX           0xC2
#define DEVICE_ID_MACRONIX_512K     0x1C
#define DEVICE_ID_MACRONIX_1M       0x09

static const u32 sIdentifyFlashV120Sig[] = { 0xB082B580u, 0x480E466Fu, 0x880A490Du, 0x1C114B0Du };
static const u32 sIdentifyFlashV123Sig[] = { 0x4A07B510u, 0x49078810u, 0x21034008u, 0x80104308u };
static const u32 sIdentifyFlash1MV103Sig[] = { 0x4A07B510u, 0x49078810u, 0x21034008u, 0x80104308u };
static const u32 sVerifyFlashV126Sig[] = { 0xB0C0B570u, 0x1C161C0Du, 0x0C040400u, 0x88104A07u };
static const u32 sReadFlash512V130Sig[] = { 0xB0A0B5F0u, 0x1C161C0Du, 0x04031C1Fu, 0x4A0F0C1Cu };
static const u32 sVerifyFlash512V130Sig[] = { 0xB0C0B570u, 0x1C161C0Du, 0x0C140402u, 0x6800480Eu };
static const u32 sVerifyFlashSector512V130Sig[] = { 0xB0C0B530u, 0x04031C0Du, 0x4A0F0C1Cu, 0x490F8810u };

struct flash_patchinfo_t
{
    u32* progSectorPtr;
    u32* progBytePtr;
    u32* eraseChipPtr;
    u32* eraseSectorPtr;
    u32* pollingSrPtr;
    u32* flMaxTimePtr;
    u32* flashPtr;
};

static flash_patchinfo_t sPatchInfo;
static flash_v120_type sFlashType;
static const u16 sMaxTime[] = { 0xA, 0xFFBD, 0xC2, 0xA, 0xFFBD, 0xC2, 0x28, 0xFFBD, 0xC2, 0xC8, 0xFFBD, 0xC2 };

static void readFlash(u16 secNo, u32 offset, u8* dst, u32 size)
{
    u32 saveAddress = (secNo << 12) + offset;
    for (u32 i = 0; i < size; ++i)
    {
        *dst++ = sav_readSaveByteFromFileFromUserMode(saveAddress++);
    }
}

static u32 verifyFlashSector(u16 secNo, const u8* src)
{
    u32 saveAddress = secNo << 12;
    for (u32 i = 0; i < (1 << 12); ++i)
    {
        u8 saveByte = sav_readSaveByteFromFileFromUserMode(saveAddress++);
        u8 expectedByte = *src++;
        if (saveByte != expectedByte)
            return 0x0E000000 + ((secNo & 0xF) << 12) + i;
    }
    return 0;
}

static u32 verifyFlash(u16 secNo, const u8* src, u32 size)
{
    u32 saveAddress = secNo << 12;
    for (u32 i = 0; i < size; ++i)
    {
        u8 saveByte = sav_readSaveByteFromFileFromUserMode(saveAddress++);
        u8 expectedByte = *src++;
        if (saveByte != expectedByte)
            return 0x0E000000 + ((secNo & 0xF) << 12) + i;
    }
    return 0;
}

static u16 eraseFlashChip()
{
    for (u32 i = 0; i < sizeof(gSaveData); ++i)
    {
        sav_writeSaveByteToFileFromUserMode(i, 0xFF);
    }
    sav_flushSaveFileFromUserMode();
    return 0;
}

static u16 eraseFlashSector(u16 secNo)
{
    for (u32 i = 0; i < (1 << 12); ++i)
    {
        sav_writeSaveByteToFileFromUserMode((secNo << 12) + i, 0xFF);
    }
    sav_flushSaveFileFromUserMode();
    return 0;
}

static u16 programFlashSector(u16 secNo, const u8* src)
{
    for (u32 i = 0; i < (1 << 12); ++i)
    {
        sav_writeSaveByteToFileFromUserMode((secNo << 12) + i, *src++);
    }
    sav_flushSaveFileFromUserMode();
    return 0;
}

static u16 programFlashByte1M(u16 secNo, u32 offset, u8 data)
{
    sav_writeSaveByteToFileFromUserMode((secNo << 12) + offset, data);
    sav_flushSaveFileFromUserMode();
    return 0;
}

static u16 identifyFlash512()
{
    *sPatchInfo.progSectorPtr = (u32)sav_callSwi8;
    *sPatchInfo.eraseChipPtr = (u32)sav_callSwi9;
    *sPatchInfo.eraseSectorPtr = (u32)sav_callSwi10;
    *sPatchInfo.pollingSrPtr = 0;
    *sPatchInfo.flMaxTimePtr = (u32)sMaxTime;
    *sPatchInfo.flashPtr = (u32)&sFlashType;
    return 0;
}

static void initializeFlash512()
{
    sav_swiTable[8] = (void*)programFlashSector;
    sav_swiTable[9] = (void*)eraseFlashChip;
    sav_swiTable[10] = (void*)eraseFlashSector;

    sFlashType.saveSize = 64 * 1024;
    sFlashType.sector.size = 0x1000;
    mem_swapByte(12, &sFlashType.sector.shift);
    sFlashType.sector.count = sFlashType.saveSize >> sFlashType.sector.shift;
    sFlashType.sector.top = 0;
    sFlashType.agbWait[0] = 0;
    sFlashType.agbWait[1] = 3;
    mem_swapByte(MAKER_ID_MACRONIX, &sFlashType.makerId);
    mem_swapByte(DEVICE_ID_MACRONIX_512K, &sFlashType.deviceId);
}

static u16 identifyFlash1M()
{
    *sPatchInfo.progSectorPtr = (u32)sav_callSwi8;
    if (sPatchInfo.progBytePtr)
    {
        *sPatchInfo.progBytePtr = (u32)sav_callSwi9;
    }
    *sPatchInfo.eraseChipPtr = (u32)sav_callSwi10;
    *sPatchInfo.eraseSectorPtr = (u32)sav_callSwi11;
    *sPatchInfo.pollingSrPtr = 0;
    *sPatchInfo.flMaxTimePtr = (u32)sMaxTime;
    *sPatchInfo.flashPtr = (u32)&sFlashType;
    return 0;
}

static void initializeFlash1M()
{
    sav_swiTable[8] = (void*)programFlashSector;
    sav_swiTable[9] = (void*)programFlashByte1M;
    sav_swiTable[10] = (void*)eraseFlashChip;
    sav_swiTable[11] = (void*)eraseFlashSector;

    sFlashType.saveSize = 128 * 1024;
    sFlashType.sector.size = 0x1000;
    mem_swapByte(12, &sFlashType.sector.shift);
    sFlashType.sector.count = sFlashType.saveSize >> sFlashType.sector.shift;
    sFlashType.sector.top = 0;
    sFlashType.agbWait[0] = 0;
    sFlashType.agbWait[1] = 3;
    mem_swapByte(MAKER_ID_MACRONIX, &sFlashType.makerId);
    mem_swapByte(DEVICE_ID_MACRONIX_1M, &sFlashType.deviceId);
}

static bool loadDataV120(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (f_lseek(romFile, tagRomAddress + ((saveTypeInfo->tagLength + 3) & ~3)) != FR_OK)
    {
        return false;
    }
    UINT read;
    if (f_read(romFile, tempBuffer, 0x94, &read) != FR_OK || read != 0x94)
    {
        return false;
    }

    sPatchInfo.progSectorPtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_PROG_SECTOR);
    sPatchInfo.progBytePtr = nullptr;
    sPatchInfo.eraseChipPtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_ERASE_CHIP);
    sPatchInfo.eraseSectorPtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_ERASE_SECTOR);
    sPatchInfo.pollingSrPtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_POLLING_SR);
    sPatchInfo.flMaxTimePtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_FL_MAXTIME);
    sPatchInfo.flashPtr = *(u32**)(tempBuffer + FLASH_V120_OFFSET_FLASH);
    return true;
}

bool flash_patchV120(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (!loadDataV120(saveTypeInfo, romFile, tagRomAddress, tempBuffer))
    {
        return false;
    }

    u32* pIdentify = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, sIdentifyFlashV120Sig);
    if (!pIdentify)
    {
        return false;
    }

    sav_swiTable[0] = (void*)identifyFlash512;
    *(u16*)pIdentify = SAVE_THUMB_SWI(0);

    sav_swiTable[1] = (void*)readFlash;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_READ) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(1);

    sav_swiTable[2] = (void*)verifyFlashSector;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(2);

    initializeFlash512();
    return true;
}

bool flash_patchV123(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (!loadDataV120(saveTypeInfo, romFile, tagRomAddress, tempBuffer))
    {
        return false;
    }

    u32* pIdentify = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, sIdentifyFlashV123Sig);
    if (!pIdentify)
    {
        return false;
    }

    sav_swiTable[0] = (void*)identifyFlash512;
    *(u16*)pIdentify = SAVE_THUMB_SWI(0);

    sav_swiTable[1] = (void*)readFlash;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_READ) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(1);

    sav_swiTable[2] = (void*)verifyFlashSector;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(2);

    initializeFlash512();
    return true;
}

bool flash_patchV126(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (!loadDataV120(saveTypeInfo, romFile, tagRomAddress, tempBuffer))
    {
        return false;
    }

    u32* pIdentify = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, sIdentifyFlashV123Sig);
    if (!pIdentify)
    {
        return false;
    }

    sav_swiTable[0] = (void*)identifyFlash512;
    *(u16*)pIdentify = SAVE_THUMB_SWI(0);

    sav_swiTable[1] = (void*)readFlash;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_READ) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(1);

    sav_swiTable[2] = (void*)verifyFlashSector;
    *(u16*)((*(u32*)(tempBuffer + FLASH_V120_OFFSET_VERIFY_SECTOR) & ~1) - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = SAVE_THUMB_SWI(2);

    if (sav_tryPatchFunction(sVerifyFlashV126Sig, 3, (void*)verifyFlash))
    {
        initializeFlash512();
        return true;
    }
    return false;
}

bool flash_patch512V130(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (f_lseek(romFile, tagRomAddress + ((saveTypeInfo->tagLength + 3) & ~3)) != FR_OK)
    {
        return false;
    }
    UINT read = 0;
    if (f_read(romFile, tempBuffer, 0x94, &read) != FR_OK || read != 0x94)
    {
        return false;
    }

    sPatchInfo.progSectorPtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_PROG_SECTOR);
    sPatchInfo.progBytePtr = nullptr;
    sPatchInfo.eraseChipPtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_ERASE_CHIP);
    sPatchInfo.eraseSectorPtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_ERASE_SECTOR);
    sPatchInfo.pollingSrPtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_POLLING_SR);
    sPatchInfo.flMaxTimePtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_FL_MAXTIME);
    sPatchInfo.flashPtr = *(u32**)(tempBuffer + FLASH_512V130_OFFSET_FLASH);

    if (sav_tryPatchFunction(sIdentifyFlashV123Sig, 0, (void*)identifyFlash512) &&
        sav_tryPatchFunction(sReadFlash512V130Sig, 1, (void*)readFlash) &&
        sav_tryPatchFunction(sVerifyFlashSector512V130Sig, 2, (void*)verifyFlashSector) &&
        sav_tryPatchFunction(sVerifyFlash512V130Sig, 3, (void*)verifyFlash))
    {
        initializeFlash512();
        return true;
    }
    return false;
}

bool flash_patch1MV102(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (f_lseek(romFile, tagRomAddress + ((saveTypeInfo->tagLength + 3) & ~3)) != FR_OK)
    {
        return false;
    }
    UINT read = 0;
    if (f_read(romFile, tempBuffer, 0x94, &read) != FR_OK || read != 0x94)
    {
        return false;
    }

    sPatchInfo.progSectorPtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_PROG_SECTOR);
    sPatchInfo.progBytePtr = nullptr;
    sPatchInfo.eraseChipPtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_ERASE_CHIP);
    sPatchInfo.eraseSectorPtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_ERASE_SECTOR);
    sPatchInfo.pollingSrPtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_POLLING_SR);
    sPatchInfo.flMaxTimePtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_FL_MAXTIME + (
        saveTypeInfo->type == SAVE_TYPE_FLASH1M_V102 ? 0 : 4));
    sPatchInfo.flashPtr = *(u32**)(tempBuffer + FLASH_1MV102_OFFSET_FLASH + (
        saveTypeInfo->type == SAVE_TYPE_FLASH1M_V102 ? 0 : 4));

    const u32* identifySignature = saveTypeInfo->type == SAVE_TYPE_FLASH1M_V102
        ? sIdentifyFlashV123Sig : sIdentifyFlash1MV103Sig;

    if (sav_tryPatchFunction(identifySignature, 0, (void*)identifyFlash1M) &&
        sav_tryPatchFunction(sReadFlash512V130Sig, 1, (void*)readFlash) &&
        sav_tryPatchFunction(sVerifyFlashSector512V130Sig, 2, (void*)verifyFlashSector) &&
        sav_tryPatchFunction(sVerifyFlash512V130Sig, 3, (void*)verifyFlash))
    {
        initializeFlash1M();
        return true;
    }
    return false;
}

bool flash_patch1MV103(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    if (f_lseek(romFile, tagRomAddress + ((saveTypeInfo->tagLength + 3) & ~3)) != FR_OK)
    {
        return false;
    }
    UINT read = 0;
    if (f_read(romFile, tempBuffer, 0x94, &read) != FR_OK || read != 0x94)
    {
        return false;
    }

    sPatchInfo.progSectorPtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_PROG_SECTOR);
    sPatchInfo.progBytePtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_PROG_BYTE);
    sPatchInfo.eraseChipPtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_ERASE_CHIP);
    sPatchInfo.eraseSectorPtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_ERASE_SECTOR);
    sPatchInfo.pollingSrPtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_POLLING_SR);
    sPatchInfo.flMaxTimePtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_FL_MAXTIME);
    sPatchInfo.flashPtr = *(u32**)(tempBuffer + FLASH_1MV103_OFFSET_FLASH);

    if (sav_tryPatchFunction(sIdentifyFlash1MV103Sig, 0, (void*)identifyFlash1M) &&
        sav_tryPatchFunction(sReadFlash512V130Sig, 1, (void*)readFlash) &&
        sav_tryPatchFunction(sVerifyFlashSector512V130Sig, 2, (void*)verifyFlashSector) &&
        sav_tryPatchFunction(sVerifyFlash512V130Sig, 3, (void*)verifyFlash))
    {
        initializeFlash1M();
        return true;
    }
    return false;
}
