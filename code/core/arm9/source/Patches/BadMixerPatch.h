#pragma once

/// @brief Patch to fix a soundmixer that has ldm with writeback and rb in rlist.
class BadMixerPatch
{
public:
    /// @brief Tries to apply the patch.
    /// @return True if the patch was successfully applied, or false otherwise.
    bool TryApplyPatch();
};
