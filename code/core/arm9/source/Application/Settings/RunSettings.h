#pragma once
#include "common.h"
#include <memory>

class RunSettings
{
public:
    /// @brief Specifies the rom addresses of instructions that should be patched by the JIT.
    std::unique_ptr<u32[]> jitPatchAddresses;

    /// @brief Specifies the number of instructions that should be patched by the JIT.
    u32 jitPatchAddressCount = 0;

    /// @brief Specifies whether the instruction cache should be enabled on iwram and ewram.
    bool16 enableWramInstructionCache = true;

    /// @brief Specifies whether the data cache should be enabled on ewram.
    bool16 enableEWramDataCache = true;

    /// @brief Specifies the rom addresses of instructions that perform a self-modifying write and that should be patched.
    std::unique_ptr<u32[]> selfModifyingPatchAddresses;

    /// @brief Specifies the number of self-modifying write instructions that should be patched.
    u32 selfModifyingPatchAddressCount = 0;
};
