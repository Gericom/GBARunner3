#include <nds.h>
#include "Environment.h"

u32 Environment::_flags;

static bool detectIsNitroEmulator()
{
    u32 agbMemoryAddress = *(vu32*)0x027FFF7C;
    if (agbMemoryAddress < 0x08000000 || agbMemoryAddress >= 0x0A000000)
        return false;
    u32 monitorRomLoadAddress = *(vu32*)0x027FFF68;
    if (monitorRomLoadAddress < 0x02000000 || monitorRomLoadAddress >= 0x02800000)
        return false;
    return true;
}

static bool detectNocashPrintSuppport()
{
    u32 nocashIdentifier = *(vu32*)0x04FFFA00;
    return nocashIdentifier == 0x67246F6E; //no$g
}

void Environment::Initialize()
{
    _flags = ENVIRONMENT_FLAGS_NONE;
    if (((*(vu32*)0x04004000) & 3) == 1)
    {
        _flags |= ENVIRONMENT_FLAGS_DSI_MODE;
    }
    else
    {
        if (detectIsNitroEmulator())
        {
            _flags |= ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR;
            _flags |= ENVIRONMENT_FLAGS_JTAG_SEMIHOSTING;

            REG_EXMEMCNT &= ~0xFF;

            u32 agbMemoryAddress = *(vu32*)0x027FFF7C;
            if (*(vu32*)(agbMemoryAddress + 0x100) == 0x44495349) //ISID
                _flags |= ENVIRONMENT_FLAGS_AGB_SEMIHOSTING;
        }
    }
    if (!(_flags & ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR))
    {
        if (detectNocashPrintSuppport())
            _flags |= ENVIRONMENT_FLAGS_NOCASH_PRINT;
    }
}