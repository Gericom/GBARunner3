#include "common.h"
#include "SdCache/SdCache.h"
#include "Application/Settings/RunSettings.h"
#include "PatchSwi.h"
#include "MemoryEmulator/RomDefs.h"
#include "SelfModifyingPatches.h"

[[gnu::section(".itcm")]]
static u32 sPatchCodeBuffer[16 * 3];

void SelfModifyingPatches::PatchSelfModifyingWrite(u32 gbaAddress)
{
    u32* instructionPtr = (u32*)sdc_loadRomBlockForPatching(gbaAddress);
    u32 instruction = *instructionPtr;
    if ((instruction & 0x0F3F0000) == 0x050F0000)
    {
        if (!ModifyStr(instruction))
            return;
    }
    else if ((instruction & 0x0F7F00F0) == 0x014F00B0)
    {
        if (!ModifyStrh(instruction))
            return;
    }

    u32 condition = instruction & (0xF << 28);
    // always condition
    instruction &= ~(0xF << 28);
    instruction |= (0xE << 28);

    auto patchCode = &sPatchCodeBuffer[_patchCodeOffset];
    _patchCodeOffset += 3;
    patchCode[0] = instruction;
    patchCode[1] = 0xEE070F15; // mcr p15, 0, r0, c7, c5, 0
    patchCode[2] = 0xE1B0F00E; // movs pc, lr

    u32 patchInstruction = ARM_PATCH_SWI(patch_addSwiPatch(patchCode));
    patchInstruction &= ~(0xF << 28);
    patchInstruction |= condition;
    if (gbaAddress >= ROM_LINEAR_GBA_ADDRESS && gbaAddress < ROM_LINEAR_END_GBA_ADDRESS)
    {
        *(u32*)(gbaAddress - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS) = patchInstruction;
    }
    *instructionPtr = patchInstruction;
}

bool SelfModifyingPatches::ModifyStr(u32& instruction)
{
    // str{b} Rd, [pc, +/-#imm]
    int offset = instruction & 0xFFF;
    if (!(instruction & (1 << 23)))
    {
        offset = -offset;
    }
    offset += 8; // because pc + 8
    offset -= 4; // because swi lr is pc + 4
    // up/down
    instruction &= ~(1 << 23);
    if (offset >= 0)
    {
        instruction |= (1 << 23);
    }
    else
    {
        offset = -offset;
    }
    // offset
    if (offset > 0xFFF)
    {
        gLogger->Log(LogLevel::Error, "Offset out of range for self-modifying instruction patch.\n");
        return false;
    }
    instruction &= ~0xFFF;
    instruction |= (offset & 0xFFF);
    // Rn
    instruction &= ~(0xF << 16);
    instruction |= (0xE << 16);
    return true;
}

bool SelfModifyingPatches::ModifyStrh(u32& instruction)
{
    // strh Rd, [pc, +/-#imm]
    int offset = (instruction & 0xF) | ((instruction & 0xF00) >> 4);
    if (!(instruction & (1 << 23)))
    {
        offset = -offset;
    }
    offset += 8; // because pc + 8
    offset -= 4; // because swi lr is pc + 4
    // up/down
    instruction &= ~(1 << 23);
    if (offset >= 0)
    {
        instruction |= (1 << 23);
    }
    else
    {
        offset = -offset;
    }
    // offset
    if (offset > 0xFF)
    {
        gLogger->Log(LogLevel::Error, "Offset out of range for self-modifying instruction patch.\n");
        return false;
    }
    instruction &= ~0xF0F;
    instruction |= (offset & 0xF) | ((offset & 0xF0) << 4);
    // Rn
    instruction &= ~(0xF << 16);
    instruction |= (0xE << 16);
    return true;
}

void SelfModifyingPatches::ApplyPatches(const RunSettings& runSettings)
{
    gLogger->Log(LogLevel::Debug, "Applying game self-modifying patches...\n");
    for (u32 i = 0; i < runSettings.selfModifyingPatchAddressCount; ++i)
    {
        u32 address = runSettings.selfModifyingPatchAddresses[i];
        if (address >= 0x08000000u && address < 0x0A000000u)
        {
            gLogger->Log(LogLevel::Debug, "0x%08X\n", address);
            PatchSelfModifyingWrite(address);
        }
    }
}
