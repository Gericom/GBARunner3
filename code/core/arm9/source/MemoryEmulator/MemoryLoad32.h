#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Loads a 32-bit value from the given GBA memory address.
///        When unaligned rotation is applied.
/// @param address The address to load from.
/// @return The loaded value.
extern u32 memu_load32FromC(u32 address);

#ifdef __cplusplus
}
#endif
