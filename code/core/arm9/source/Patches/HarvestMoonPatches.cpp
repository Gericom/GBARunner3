#include "common.h"
#include "GameCode.h"
#include "SdCache/SdCache.h"
#include "HarvestMoonPatches.h"

bool HarvestMoonPatches::TryApplyPatches(u32 gameCode)
{
    // patch return address in rom check (cmp lr, #0x08000000)
    u32 patchAddress;
    switch (gameCode)
    {
        case GAMECODE("A4ND"): // Harvest Moon - Friends of Mineral Town (Germany)
        {
            patchAddress = 0x0874C3C8;
            break;
        }
        case GAMECODE("A4NE"): // Harvest Moon - Friends of Mineral Town (USA)
        {
            patchAddress = 0x0875BF84;
            break;
        }
        case GAMECODE("A4NJ"): // Bokujou Monogatari - Mineral Town no Nakama-tachi (Japan)
        {
            patchAddress = 0x0877FB9C;
            break;
        }
        case GAMECODE("A4NP"): // Harvest Moon - Friends of Mineral Town (Europe)
        {
            patchAddress = 0x0875BFE0;
            break;
        }
        case GAMECODE("BFGE"): // Harvest Moon - More Friends of Mineral Town  (USA)
        {
            patchAddress = 0x089D9608;
            break;
        }
        case GAMECODE("BFGJ"): // Bokujou Monogatari - Mineral Town no Nakama-tachi for Girl (Japan).gba
        {
            patchAddress = 0x089E07F0;
            break;
        }
        default:
        {
            return false;
        }
    }
    *(u32*)sdc_loadRomBlockForPatching(patchAddress) = 0xE35E0000; // cmp lr, #0
    return true;
}
