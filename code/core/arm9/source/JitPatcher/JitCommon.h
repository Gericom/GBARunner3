#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Gets a pointer to the word containing the JIT bits for the given address.
/// @param ptr The address.
/// @return A pointer to the word containing the JIT bits for the given address.
u32* jit_getJitBits(const void* ptr);

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

bool jit_conditionPass(u32 cpsr, u32 condition);

/// @brief Initializes the JIT patcher.
void jit_init(void);

#ifdef __cplusplus
}
#endif
