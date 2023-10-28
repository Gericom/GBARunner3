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
};
