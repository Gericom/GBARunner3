#include "common.h"
#include <string.h>
#include "tlsf.h"
#include "GlobalHeap.h"

static tlsf_t sHeap;

extern "C" void* malloc(size_t size)
{
    return tlsf_malloc(sHeap, size);
}

extern "C" void* _malloc_r(struct _reent *, size_t size)
{
    return malloc(size);
}

extern "C" void free(void* ptr)
{
    return tlsf_free(sHeap, ptr);
}

extern "C" void _free_r(struct _reent *, void* ptr)
{
    free(ptr);
}

extern "C" void* realloc(void* ptr, size_t size)
{
    return tlsf_realloc(sHeap, ptr, size);
}

extern "C" void* memalign(size_t alignment, size_t size)
{
    return tlsf_memalign(sHeap, alignment, size);
}

void* operator new(std::size_t blocksize)
{
    return malloc(blocksize);
}

void* operator new(std::size_t size, std::align_val_t al)
{
    return memalign(static_cast<std::size_t>(al), size);
}

void* operator new[](std::size_t blocksize)
{
    return malloc(blocksize);
}

void* operator new[](std::size_t size, std::align_val_t al)
{
    return memalign(static_cast<std::size_t>(al), size);
}

void operator delete(void* ptr)
{
    free(ptr);
}

void operator delete(void* ptr, std::align_val_t align)
{
    free(ptr);
}

void operator delete(void* ptr, std::size_t size, std::align_val_t align)
{
    free(ptr);
}

void operator delete[](void* ptr)
{
    free(ptr);
}

void operator delete[](void* ptr, std::align_val_t align)
{
    free(ptr);
}

void operator delete[](void* ptr, std::size_t size, std::align_val_t align)
{
    free(ptr);
}

extern u8 __heap_start[];
extern u8 __heap_end[];

void heap_init()
{
    u32 heapStart = (u32)__heap_start;
    heapStart = (heapStart + 31) & ~31;
    u32 heapEnd = (u32)__heap_end;
    heapEnd = heapEnd & ~31;
    u32 tlsfSize = tlsf_size();
    memset((void*)heapStart, 0, tlsfSize);
    memset((u8*)heapStart + tlsfSize, 0xA5, heapEnd - heapStart - tlsfSize);
    sHeap = tlsf_create_with_pool((void*)heapStart, heapEnd - heapStart);
}
