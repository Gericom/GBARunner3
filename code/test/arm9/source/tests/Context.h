#pragma once

struct context_t
{
    u32 r[15];
};

#define EXPECT_CONTEXT_EQ(actual, expected) do { \
    EXPECT_THAT((actual).r[0], Eq((expected).r[0]));\
    EXPECT_THAT((actual).r[1], Eq((expected).r[1]));\
    EXPECT_THAT((actual).r[2], Eq((expected).r[2]));\
    EXPECT_THAT((actual).r[3], Eq((expected).r[3]));\
    EXPECT_THAT((actual).r[4], Eq((expected).r[4]));\
    EXPECT_THAT((actual).r[5], Eq((expected).r[5]));\
    EXPECT_THAT((actual).r[6], Eq((expected).r[6]));\
    EXPECT_THAT((actual).r[7], Eq((expected).r[7]));\
    EXPECT_THAT((actual).r[8], Eq((expected).r[8]));\
    EXPECT_THAT((actual).r[9], Eq((expected).r[9]));\
    EXPECT_THAT((actual).r[10], Eq((expected).r[10]));\
    EXPECT_THAT((actual).r[11], Eq((expected).r[11]));\
    EXPECT_THAT((actual).r[12], Eq((expected).r[12]));\
    EXPECT_THAT((actual).r[13], Eq((expected).r[13]));\
    EXPECT_THAT((actual).r[14], Eq((expected).r[14])); } while (0)

constexpr const context_t gRandomContext
{ 
    0x1b060e8d,
    0xfb27d1f0,
    0xe5b63373,
    0x952a55c5,
    0x7f2631fa,
    0xdbcf199c,
    0x06e2ff61,
    0xebe93024,
    0x01bfb265,
    0x32b5b791,
    0xfd0e3f8b,
    0x6a60e7cf,
    0x759fb795,
    0xbe3836af,
    0x2367f028
};

// static void dumpContext(const context_t* context)
// {
//     for (u32 i = 0; i < 15; i++)
//     {
//         LOG_DEBUG("R%d: 0x%x\n", i, context->r[i]);
//     }
// }
