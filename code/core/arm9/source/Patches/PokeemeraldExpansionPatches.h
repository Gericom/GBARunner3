#pragma once
#include "Application/Settings/RunSettings.h"

/// @brief Fixes up the run settings for hacks based on pokeemerald-expansion. These keep the game code
///        of Pokémon Emerald, but have a different crt0 layout than the original game.
class PokeemeraldExpansionPatches
{
public:
    /// @brief Tries to apply the patches.
    /// @param runSettings The run settings to fix up.
    /// @return True if the patches were successfully applied, or false otherwise.
    bool TryApplyPatches(RunSettings& runSettings);

private:
    static bool IsPsrTransfer(u32 instruction);
};
