#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Invalidates the entire instruction cache.
static inline void ic_invalidateAll()
{
    asm volatile("mcr p15, 0, %0, c7, c5, 0\n" :: "r"(0));
}

/// @brief Drains the write buffer.
static inline void dc_drainWriteBuffer()
{
    asm volatile("mcr p15, 0, %0, c7, c10, 4\n" :: "r"(0));
}

/// @brief Flushes the data cache in the given range. The effected cache lines will be marked as invalid.
///        Ptr should be 32-byte aligned. ByteCount is rounded up to 32-byte multiples.
/// @param ptr A pointer to the memory block to flush. Should be 32-byte aligned.
/// @param byteCount The number of bytes to flush. Will be rounded up to 32-byte multiples.
extern void dc_flushRange(const void* ptr, u32 byteCount);

/// @brief Invalidates the data cache in the given range.
/// @param ptr A pointer to the memory block to invalidate. Should be 32-byte aligned.
/// @param byteCount The number of bytes to invalidate. Will be rounded up to 32-byte multiples.
extern void dc_invalidateRange(void* ptr, u32 byteCount);

/// @brief Invalidates a single cache line.
/// @param ptr A pointer to an address in the cache line to invalidate.
static inline void dc_invalidateLine(void* ptr)
{
    asm volatile("mcr p15, 0, %0, c7, c6, 1\n" :: "r"(((u32)ptr) & ~0x1F));
}

/// @brief Halts the processor until an interrupt occurs.
static inline void arm_waitForInterrupt(void)
{
    asm volatile("mcr p15, 0, %0, c7, c0, 4\n" :: "r"(0));
}

#ifdef __cplusplus
}
#endif
