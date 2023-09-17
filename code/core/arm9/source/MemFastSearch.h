#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Searched for the given 32 bit aligned 16 byte pattern in the given data with
///        length dataLength. The 4 words of the pattern are assumed to be unique.
/// @param data A 32 bit aligned pointer to the data to search in.
/// @param dataLength The length of the data to search in in bytes.
/// @param pattern The 16 byte pattern to search for.
/// @return When successful returns a pointer to the found pattern in data, or nullptr otherwise.
extern const u32* mem_fastSearch16(const u32* data, u32 dataLength, const u32* pattern);

#ifdef __cplusplus
}
#endif
