#pragma once

typedef struct
{
    /// @brief Stores for each halfword in the statically loaded part of the rom
    ///        whether it was processed by the JIT (1) or not (0).
    u32 staticRomJitBits[(2 * 1024 * 1024) / 2 / 32];

    /// @brief Stores for each halfword in IWRAM whether it was processed by the JIT (1) or not (0).
    u32 iWramJitBits[(32 * 1024) / 2 / 32];

    /// @brief Stores for each halfword in the dynamically loaded part of the rom (sd cache)
    ///        whether it was processed by the JIT (1) or not (0).
    u32 dynamicRomJitBits[SDC_SIZE / 2 / 32];

    /// @brief Stores for each halfword in EWRAM whether it was processed by the JIT (1) or not (0).
    u32 eWramJitBits[(256 * 1024) / 2 / 32];

    /// @brief Stores for each halfword in VRAM whether it was processed by the JIT (1) or not (0).
    u32 vramJitBits[(96 * 1024) / 2 / 32];

    u32 dummyJitBits;

    /// @brief Stores 2 auxillary bits for each halfword in the statically loaded part of the rom.
    u32 staticRomJitAuxBits[(2 * 1024 * 1024) / 32];

    /// @brief Stores 2 auxillary bits for each halfword in IWRAM.
    u32 iWramJitAuxBits[(32 * 1024) / 32];

    /// @brief Stores 2 auxillary bits for each halfword in the dynamically loaded part of the rom (sd cache).
    u32 dynamicRomJitAuxBits[SDC_SIZE / 32];

    /// @brief Stores 2 auxillary bits for each halfword in EWRAM.
    u32 eWramJitAuxBits[(256 * 1024) / 32];

    /// @brief Stores 2 auxillary bits for each halfword in VRAM.
    u32 vramJitAuxBits[(96 * 1024) / 32];

    u32 dummyJitAuxBits;
} jit_state_t;

extern jit_state_t gJitState;

#ifdef __cplusplus
extern "C" {
#endif

[[gnu::pure]]
u32 jit_getJitBitsOffset(const void* ptr);

/// @brief Gets a pointer to the word containing the JIT bits for the given address.
/// @param ptr The address.
/// @return A pointer to the word containing the JIT bits for the given address.
static inline u8* jit_getJitBits(const void* ptr)
{
    u32 offset = jit_getJitBitsOffset(ptr);
    return (u8*)gJitState.staticRomJitBits + offset;
}

/// @brief Gets a pointer to the word containing the JIT auxillary bits for the given address.
/// @param ptr The address.
/// @return A pointer to the word containing the JIT auxillary bits for the given address.
static inline u16* jit_getJitAuxBits(const void* ptr)
{
    u32 offset = jit_getJitBitsOffset(ptr);
    return (u16*)gJitState.staticRomJitAuxBits + offset;
}

/// @brief Finds the start of the block (continous region) containing the given address.
/// @param ptr The address.
/// @return A pointer to the start of the block that contains the given address.
void* jit_findBlockStart(const void* ptr);

/// @brief Finds the exclusive end of the block (continous region) containing the given address.
/// @param ptr The address.
/// @return A pointer to the exclusive end of the block that contains the given address.
void* jit_findBlockEnd(const void* ptr);

bool jit_isBlockJitted(void* ptr);
void jit_ensureBlockJitted(void* ptr);

/// @brief Initializes the JIT patcher.
void jit_init(void);

#ifdef __cplusplus
}
#endif
