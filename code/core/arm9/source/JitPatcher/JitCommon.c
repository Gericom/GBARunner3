#include "common.h"
#include <string.h>
#include "SdCache/SdCache.h"
#include "cp15.h"
#include "JitArm.h"
#include "JitThumb.h"
#include "JitCommon.h"

[[gnu::section(".ewram.bss")]]
jit_state_t gJitState;

[[gnu::section(".itcm"), gnu::optimize("Oz")]]
u32 jit_getJitBitsOffset(const void* ptr)
{
    u32 jitBitsOffset;
    u32 offset;

    if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        jitBitsOffset = offsetof(jit_state_t, staticRomJitBits);
        offset = (u32)ptr - 0x02200000;
    }
    else if ((u32)ptr >= 0x03000000 && (u32)ptr < 0x03008000)
    {
        // IWRAM
        jitBitsOffset = offsetof(jit_state_t, iWramJitBits);
        offset = (u32)ptr - 0x03000000;
    }
    else if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        jitBitsOffset = offsetof(jit_state_t, dynamicRomJitBits);
        offset = (u32)ptr - (u32)sdc_cache;
    }
    else if ((u32)ptr >= 0x02000000 && (u32)ptr < 0x02040000)
    {
        // EWRAM
        jitBitsOffset = offsetof(jit_state_t, eWramJitBits);
        offset = (u32)ptr - 0x02000000;
    }
    else if ((u32)ptr >= 0x06000000 && (u32)ptr < 0x06018000)
    {
        // VRAM
        jitBitsOffset = offsetof(jit_state_t, vramJitBits);
        offset = (u32)ptr - 0x06000000;
    }
    else
    {
        jitBitsOffset = offsetof(jit_state_t, dummyJitBits);
        offset = 0;
    }

    return jitBitsOffset + (offset / 2 / 8);
}

void* jit_findBlockStart(const void* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (void*)((u32)ptr & ~SDC_BLOCK_MASK);
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (void*)0x02200000;
    }
    // no significant block boundary
    return (void*)0;
}

void* jit_findBlockEnd(const void* ptr)
{
    if ((u32)ptr >= (u32)sdc_cache && (u32)ptr < (u32)sdc_cache[SDC_BLOCK_COUNT])
    {
        // sd cache
        return (void*)(((u32)ptr & ~SDC_BLOCK_MASK) + SDC_BLOCK_SIZE);
    }
    else if ((u32)ptr >= 0x02200000 && (u32)ptr < 0x02400000)
    {
        // static rom region
        return (void*)0x02400000;
    }
    // no significant block boundary
    return (void*)0xFFFFFFFF;
}

[[gnu::section(".itcm")]]
bool jit_isBlockJitted(void* ptr)
{
    if ((u32)ptr >= 0x08000000)
    {
        ptr = (void*)((u32)ptr - 0x08000000 + 0x02200000);
    }

    const u8* const jitBits = jit_getJitBits(ptr);
    u32 bitIdx = ((u32)ptr & 0xF) >> 1;
    return (*jitBits >> bitIdx) & 1;
}

[[gnu::section(".itcm")]]
void jit_ensureBlockJitted(void* ptr)
{
    if ((u32)ptr >= 0x08000000)
    {
        ptr = (void*)((u32)ptr - 0x08000000 + 0x02200000);
    }

    const u8* const jitBits = jit_getJitBits(ptr);
    u32 bitIdx = ((u32)ptr & 0xF) >> 1;
    if ((*jitBits >> bitIdx) & 1)
        return;
    if ((u32)ptr & 1)
    {
        jit_processThumbBlock((u16*)((u32)ptr & ~1));
    }
    else
    {
        jit_processArmBlock((u32*)ptr);
    }
    dc_drainWriteBuffer();
    ic_invalidateAll();
}

void jit_init(void)
{
    memset(&gJitState, 0, sizeof(gJitState));
    gJitState.dummyJitBits = ~0u;
}

extern u32 jit_armUndefinedBxR0[];
extern u32 jit_armUndefinedBxR1[];
extern u32 jit_armUndefinedBxR2[];
extern u32 jit_armUndefinedBxR3[];
extern u32 jit_armUndefinedBxR4[];
extern u32 jit_armUndefinedBxR5[];
extern u32 jit_armUndefinedBxR6[];
extern u32 jit_armUndefinedBxR7[];
extern u32 jit_armUndefinedBxR8[];
extern u32 jit_armUndefinedBxR9[];
extern u32 jit_armUndefinedBxR10[];
extern u32 jit_armUndefinedBxR11[];
extern u32 jit_armUndefinedBxR12[];
extern u32 jit_armUndefinedBxR13[];
extern u32 jit_armUndefinedBxR14[];
extern u32 jit_armUndefinedBxR15[];
extern void jit_armPlaceBackBxR0();
extern void jit_armPlaceBackBxR1();
extern void jit_armPlaceBackBxR2();
extern void jit_armPlaceBackBxR3();
extern void jit_armPlaceBackBxR4();
extern void jit_armPlaceBackBxR5();
extern void jit_armPlaceBackBxR6();
extern void jit_armPlaceBackBxR7();
extern void jit_armPlaceBackBxR8();
extern void jit_armPlaceBackBxR9();
extern void jit_armPlaceBackBxR10();
extern void jit_armPlaceBackBxR11();
extern void jit_armPlaceBackBxR12();
extern void jit_armPlaceBackBxR13();
extern void jit_armPlaceBackBxR14();
extern void jit_armPlaceBackBxR15();

static void setupPlaceBackArmBx(void)
{
    jit_armUndefinedBxR0[0] = 0xE51FF004;
    jit_armUndefinedBxR0[1] = (u32)jit_armPlaceBackBxR0;
    jit_armUndefinedBxR1[0] = 0xE51FF004;
    jit_armUndefinedBxR1[1] = (u32)jit_armPlaceBackBxR1;
    jit_armUndefinedBxR2[0] = 0xE51FF004;
    jit_armUndefinedBxR2[1] = (u32)jit_armPlaceBackBxR2;
    jit_armUndefinedBxR3[0] = 0xE51FF004;
    jit_armUndefinedBxR3[1] = (u32)jit_armPlaceBackBxR3;
    jit_armUndefinedBxR4[0] = 0xE51FF004;
    jit_armUndefinedBxR4[1] = (u32)jit_armPlaceBackBxR4;
    jit_armUndefinedBxR5[0] = 0xE51FF004;
    jit_armUndefinedBxR5[1] = (u32)jit_armPlaceBackBxR5;
    jit_armUndefinedBxR6[0] = 0xE51FF004;
    jit_armUndefinedBxR6[1] = (u32)jit_armPlaceBackBxR6;
    jit_armUndefinedBxR7[0] = 0xE51FF004;
    jit_armUndefinedBxR7[1] = (u32)jit_armPlaceBackBxR7;
    jit_armUndefinedBxR8[0] = 0xE51FF004;
    jit_armUndefinedBxR8[1] = (u32)jit_armPlaceBackBxR8;
    jit_armUndefinedBxR9[0] = 0xE51FF004;
    jit_armUndefinedBxR9[1] = (u32)jit_armPlaceBackBxR9;
    jit_armUndefinedBxR10[0] = 0xE51FF004;
    jit_armUndefinedBxR10[1] = (u32)jit_armPlaceBackBxR10;
    jit_armUndefinedBxR11[0] = 0xE51FF004;
    jit_armUndefinedBxR11[1] = (u32)jit_armPlaceBackBxR11;
    jit_armUndefinedBxR12[0] = 0xE51FF004;
    jit_armUndefinedBxR12[1] = (u32)jit_armPlaceBackBxR12;
    jit_armUndefinedBxR13[0] = 0xE51FF004;
    jit_armUndefinedBxR13[1] = (u32)jit_armPlaceBackBxR13;
    jit_armUndefinedBxR14[0] = 0xE51FF004;
    jit_armUndefinedBxR14[1] = (u32)jit_armPlaceBackBxR14;
    jit_armUndefinedBxR15[0] = 0xE51FF004;
    jit_armUndefinedBxR15[1] = (u32)jit_armPlaceBackBxR15;
}

extern u32 jit_thumbUndefinedBxR0[];
extern u32 jit_thumbUndefinedBxR1[];
extern u32 jit_thumbUndefinedBxR2[];
extern u32 jit_thumbUndefinedBxR3[];
extern u32 jit_thumbUndefinedBxR4[];
extern u32 jit_thumbUndefinedBxR5[];
extern u32 jit_thumbUndefinedBxR6[];
extern u32 jit_thumbUndefinedBxR7[];
extern u32 jit_thumbUndefinedBxR8[];
extern u32 jit_thumbUndefinedBxR9[];
extern u32 jit_thumbUndefinedBxR10[];
extern u32 jit_thumbUndefinedBxR11[];
extern u32 jit_thumbUndefinedBxR12[];
extern u32 jit_thumbUndefinedBxR13[];
extern u32 jit_thumbUndefinedBxR14[];
extern u32 jit_thumbUndefinedBxR15[];
extern void jit_thumbPlaceBackBxR0();
extern void jit_thumbPlaceBackBxR1();
extern void jit_thumbPlaceBackBxR2();
extern void jit_thumbPlaceBackBxR3();
extern void jit_thumbPlaceBackBxR4();
extern void jit_thumbPlaceBackBxR5();
extern void jit_thumbPlaceBackBxR6();
extern void jit_thumbPlaceBackBxR7();
extern void jit_thumbPlaceBackBxR8();
extern void jit_thumbPlaceBackBxR9();
extern void jit_thumbPlaceBackBxR10();
extern void jit_thumbPlaceBackBxR11();
extern void jit_thumbPlaceBackBxR12();
extern void jit_thumbPlaceBackBxR13();
extern void jit_thumbPlaceBackBxR14();
extern void jit_thumbPlaceBackBxR15();

static void setupPlaceBackThumbBx(void)
{
    jit_thumbUndefinedBxR0[0] = 0xE51FF004;
    jit_thumbUndefinedBxR0[1] = (u32)jit_thumbPlaceBackBxR0;
    jit_thumbUndefinedBxR1[0] = 0xE51FF004;
    jit_thumbUndefinedBxR1[1] = (u32)jit_thumbPlaceBackBxR1;
    jit_thumbUndefinedBxR2[0] = 0xE51FF004;
    jit_thumbUndefinedBxR2[1] = (u32)jit_thumbPlaceBackBxR2;
    jit_thumbUndefinedBxR3[0] = 0xE51FF004;
    jit_thumbUndefinedBxR3[1] = (u32)jit_thumbPlaceBackBxR3;
    jit_thumbUndefinedBxR4[0] = 0xE51FF004;
    jit_thumbUndefinedBxR4[1] = (u32)jit_thumbPlaceBackBxR4;
    jit_thumbUndefinedBxR5[0] = 0xE51FF004;
    jit_thumbUndefinedBxR5[1] = (u32)jit_thumbPlaceBackBxR5;
    jit_thumbUndefinedBxR6[0] = 0xE51FF004;
    jit_thumbUndefinedBxR6[1] = (u32)jit_thumbPlaceBackBxR6;
    jit_thumbUndefinedBxR7[0] = 0xE51FF004;
    jit_thumbUndefinedBxR7[1] = (u32)jit_thumbPlaceBackBxR7;
    jit_thumbUndefinedBxR8[0] = 0xE51FF004;
    jit_thumbUndefinedBxR8[1] = (u32)jit_thumbPlaceBackBxR8;
    jit_thumbUndefinedBxR9[0] = 0xE51FF004;
    jit_thumbUndefinedBxR9[1] = (u32)jit_thumbPlaceBackBxR9;
    jit_thumbUndefinedBxR10[0] = 0xE51FF004;
    jit_thumbUndefinedBxR10[1] = (u32)jit_thumbPlaceBackBxR10;
    jit_thumbUndefinedBxR11[0] = 0xE51FF004;
    jit_thumbUndefinedBxR11[1] = (u32)jit_thumbPlaceBackBxR11;
    jit_thumbUndefinedBxR12[0] = 0xE51FF004;
    jit_thumbUndefinedBxR12[1] = (u32)jit_thumbPlaceBackBxR12;
    jit_thumbUndefinedBxR13[0] = 0xE51FF004;
    jit_thumbUndefinedBxR13[1] = (u32)jit_thumbPlaceBackBxR13;
    jit_thumbUndefinedBxR14[0] = 0xE51FF004;
    jit_thumbUndefinedBxR14[1] = (u32)jit_thumbPlaceBackBxR14;
    jit_thumbUndefinedBxR15[0] = 0xE51FF004;
    jit_thumbUndefinedBxR15[1] = (u32)jit_thumbPlaceBackBxR15;
}

extern u32 gGbaBios[];

void jit_disable(void)
{
    memset(gJitState.staticRomJitBits, 0xFF, sizeof(gJitState.staticRomJitBits));
    memset(gJitState.dynamicRomJitBits, 0xFF, sizeof(gJitState.dynamicRomJitBits));
    memset(gJitState.iWramJitBits, 0xFF, sizeof(gJitState.iWramJitBits));
    memset(gJitState.eWramJitBits, 0xFF, sizeof(gJitState.eWramJitBits));
    memset(gJitState.vramJitBits, 0xFF, sizeof(gJitState.vramJitBits));
    setupPlaceBackArmBx();
    setupPlaceBackThumbBx();
    gGbaBios[0x0134 >> 2] = 0xE510F004;
    dc_flushRange(gGbaBios, 16 * 1024);
    dc_drainWriteBuffer();
    ic_invalidateAll();
}
