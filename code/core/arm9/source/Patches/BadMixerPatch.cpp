#include "common.h"
#include "GameCode.h"
#include "SdCache/SdCache.h"
#include "MemFastSearch.h"
#include "BadMixerPatch.h"

static const u32 sBuggedMixerSignature[4] = { 0xE92D1FF0u, 0xE8B0000Fu, 0xE1A03203u, 0xE0822001u };

bool BadMixerPatch::TryApplyPatch()
{
    u32* function = (u32*)mem_fastSearch16((const u32*)0x02200000, 0x200000, sBuggedMixerSignature);
    if (!function)
    {
        return false;
    }

    // remove the writeback
    function[1] = 0xE890000F;
    *(u32*)sdc_loadRomBlockForPatching(((u32)&function[1]) - 0x02200000 + 0x08000000) = 0xE890000F;

    // some games contain it twice for some reason
    function = (u32*)mem_fastSearch16((const u32*)0x02200000, 0x200000, sBuggedMixerSignature);
    if (function)
    {
        function[1] = 0xE890000F;
        *(u32*)sdc_loadRomBlockForPatching(((u32)&function[1]) - 0x02200000 + 0x08000000) = 0xE890000F;
    }

    return true;
}
