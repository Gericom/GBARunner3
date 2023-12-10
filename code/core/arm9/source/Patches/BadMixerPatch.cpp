#include "common.h"
#include "SdCache/SdCache.h"
#include "MemFastSearch.h"
#include "MemoryEmulator/RomDefs.h"
#include "BadMixerPatch.h"

static const u32 sBuggedMixerSignature[4] = { 0xE92D1FF0u, 0xE8B0000Fu, 0xE1A03203u, 0xE0822001u };

bool BadMixerPatch::TryApplyPatch()
{
    u32* function = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, sBuggedMixerSignature);
    if (!function)
    {
        return false;
    }

    // remove the writeback
    function[1] = 0xE890000F;
    *(u32*)sdc_loadRomBlockForPatching(((u32)&function[1]) - ROM_LINEAR_DS_ADDRESS + ROM_LINEAR_GBA_ADDRESS) = 0xE890000F;

    // some games contain it twice for some reason
    function = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, sBuggedMixerSignature);
    if (function)
    {
        function[1] = 0xE890000F;
        *(u32*)sdc_loadRomBlockForPatching(((u32)&function[1]) - ROM_LINEAR_DS_ADDRESS + ROM_LINEAR_GBA_ADDRESS) = 0xE890000F;
    }

    return true;
}
