#pragma once
#include <cstddef>

void* operator new(std::size_t blocksize) noexcept;
void* operator new(std::size_t size, std::align_val_t al) noexcept;
void* operator new[](std::size_t blocksize) noexcept;
void* operator new[](std::size_t size, std::align_val_t al) noexcept;
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, std::align_val_t align) noexcept;
void operator delete(void* ptr, std::size_t size, std::align_val_t align) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete[](void* ptr, std::align_val_t align) noexcept;
void operator delete[](void* ptr, std::size_t size, std::align_val_t align) noexcept;

constexpr std::align_val_t cache_align { 32 };

void heap_init();
