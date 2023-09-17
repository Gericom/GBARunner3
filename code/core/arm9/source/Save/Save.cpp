#include "common.h"
#include "MemFastSearch.h"
#include "SaveSwi.h"
#include "Save.h"

[[gnu::section(".ewram.bss")]]
u8 gSaveData[SAVE_DATA_SIZE] alignas(32);

bool sav_tryPatchFunction(const u32* signature, u32 saveSwiNumber, void* patchFunction)
{
    u32* function = (u32*)mem_fastSearch16((const u32*)0x02200000, 0x200000, signature);
    if (!function)
        return false;

    sav_swiTable[saveSwiNumber] = patchFunction;
    *(u16*)function = SAVE_THUMB_SWI(saveSwiNumber);
    return true;
}
