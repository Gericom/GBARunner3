#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Copies 32-bit aligned data fast.
/// @param src The source address; must be 32-bit aligned.
/// @param dst The destination address; must be 32-bit aligned.
/// @param byteCount The number of bytes to copy; must be a multiple of 4.
extern void mem_copy32(const void* src, void* dst, u32 byteCount);

/// @brief Copies 16-bit aligned data fast.
/// @param src The source address; must be 16-bit aligned.
/// @param dst The destination address; must be 16-bit aligned.
/// @param byteCount The number of bytes to copy; must be a multiple of 2.
extern void mem_copy16(const void* src, void* dst, u32 byteCount);

#ifdef __cplusplus
}
#endif
