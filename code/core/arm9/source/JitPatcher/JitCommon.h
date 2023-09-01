#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Gets a pointer to the word containing the JIT bits for the given address.
/// @param ptr The address.
/// @return A pointer to the word containing the JIT bits for the given address.
u32* jit_getJitBits(const u32* ptr);

/// @brief Finds the start of the block (continous region) containing the given address.
/// @param ptr The address.
/// @return A pointer to the start of the block that contains the given address.
u32* jit_findBlockStart(const u32* ptr);

/// @brief Finds the exclusive end of the block (continous region) containing the given address.
/// @param ptr The address.
/// @return A pointer to the exclusive end of the block that contains the given address.
u32* jit_findBlockEnd(const u32* ptr);

/// @brief Initializes the JIT patcher.
void jit_init(void);

#ifdef __cplusplus
}
#endif
