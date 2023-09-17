#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Copies byte by byte from a privileged source to a gba destination.
/// @param src Source address.
/// @param dst Destination gba address.
/// @param byteLength Length in bytes.
extern void sav_copyBytesToGba(const u8* src, u8* dst, u32 byteLength);

/// @brief Copies byte by byte from a gba source to a privileged destination.
/// @param src Source gba address.
/// @param dst Destination address.
/// @param byteLength Length in bytes.
extern void sav_copyBytesFromGba(const u8* src, u8* dst, u32 byteLength);

/// @brief Reads a single byte from the given gba address.
/// @param src The gba address to read a byte from.
/// @return The byte that was read.
extern u8 sav_readGbaByte(const u8* src);

/// @brief Writes a single byte to the given gba address.
/// @param dst The gba address to write a byte to.
extern void sav_writeGbaByte(u8* dst, u8 value);

#ifdef __cplusplus
}
#endif