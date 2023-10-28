#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Processes one ARM instruction and applies JIT patches if needed.
/// @param ptr A pointer to the instruction to process.
/// @return True if JIT processing can continue after this instruction, or false otherwise.
bool jit_processArmInstruction(u32* ptr);

/// @brief Processes a block of ARM instructions starting at ptr.
/// @param ptr The start of the block of ARM instructions to process.
void jit_processArmBlock(u32* ptr);

#ifdef __cplusplus
}
#endif
