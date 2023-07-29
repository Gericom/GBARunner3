#include "common.h"

extern "C" unsigned int __atomic_fetch_sub_4(volatile void* ptr, unsigned int val, int memorder)
{
    unsigned int tmp = *(unsigned int*)ptr;
    *(unsigned int*)ptr -= val;
    return tmp;
}
