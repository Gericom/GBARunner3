#include "common.h"
#include <memory>
#include <string.h>
#include "MemoryEmulator/RomDefs.h"
#include "PokeemeraldExpansionPatches.h"

#define EXPANSION_HEADER_OFFSET     0x204
#define EXPANSION_HEADER_MAGIC      "RHHEXP"
#define CRT0_SCAN_LIMIT             0x400
#define CRT0_MAX_PSR_TRANSFERS      16

#define ARM_B_MASK                  0xFF000000
#define ARM_B                       0xEA000000
#define ARM_BX_LR                   0xE12FFF1E

bool PokeemeraldExpansionPatches::TryApplyPatches(RunSettings& runSettings)
{
    const u8* rom = (const u8*)ROM_LINEAR_DS_ADDRESS;
    if (memcmp(rom + EXPANSION_HEADER_OFFSET, EXPANSION_HEADER_MAGIC, sizeof(EXPANSION_HEADER_MAGIC) - 1) != 0)
    {
        return false;
    }

    // configured addresses are for the crt0 of the original game, which is shifted in expansion builds
    if (runSettings.jitPatchAddressCount == 0)
    {
        return false;
    }

    u32 entryInstruction = *(const u32*)rom;
    if ((entryInstruction & ARM_B_MASK) != ARM_B)
    {
        return false;
    }

    u32 initOffset = 8 + (((int)(entryInstruction << 8)) >> 6);
    auto addresses = std::make_unique<u32[]>(CRT0_MAX_PSR_TRANSFERS);
    u32 count = 0;
    for (u32 offset = initOffset; offset < initOffset + CRT0_SCAN_LIMIT; offset += 4)
    {
        u32 instruction = *(const u32*)(rom + offset);
        if (IsPsrTransfer(instruction))
        {
            if (count == CRT0_MAX_PSR_TRANSFERS)
            {
                return false;
            }
            addresses[count++] = ROM_LINEAR_GBA_ADDRESS + offset;
        }
        else if (instruction == ARM_BX_LR)
        {
            // end of IntrMain, which is the last arm function in crt0
            runSettings.jitPatchAddresses = std::move(addresses);
            runSettings.jitPatchAddressCount = count;
            return true;
        }
    }

    return false;
}

bool PokeemeraldExpansionPatches::IsPsrTransfer(u32 instruction)
{
    return (instruction & 0x0FBF0FFF) == 0x010F0000   // MRS
        || (instruction & 0x0FB0FFF0) == 0x0120F000   // MSR reg
        || (instruction & 0x0FB0F000) == 0x0320F000;  // MSR imm
}
