#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ArmTest.h"
#include "../ThumbTest.h"
#include "../ContextTest.h"

using namespace ::testing;

// TEST(UndefinedInstructionTests, MrsMsrSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x100000; i++)
//     {
//         bool isUndefined = test_isArmInstructionUndefined(0xE3000000u | i, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xE3000000u | i | (1 << 22), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, MrsMsrSubstituteConditionChecked)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act
//     bool isUndefined = test_isArmInstructionUndefined(0x03000000u, &inContext, &outContext);

//     // Assert
//     EXPECT_THAT(isUndefined, Eq(false));
// }

// TEST(UndefinedInstructionTests, AluSPCSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x100000; i++)
//     {
//         bool isUndefined = test_isArmInstructionUndefined(0xEC000000u | i, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC000000u | i | (1 << 21), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, LdmSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x100000; i++)
//     {
//         bool isUndefined = test_isArmInstructionUndefined(0xEC500000u | i, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 23), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 24), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 24) | (1 << 23), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));

//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 21), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 23) | (1 << 21), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 24) | (1 << 21), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC500000u | i | (1 << 24) | (1 << 23) | (1 << 21), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, StmSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x100000; i++)
//     {
//         bool isUndefined = test_isArmInstructionUndefined(0xEC400000u | i, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC400000u | i | (1 << 23), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC400000u | i | (1 << 24), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//         isUndefined = test_isArmInstructionUndefined(0xEC400000u | i | (1 << 24) | (1 << 23), &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, BSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x1000000; i++)
//     {
//         u32 hi = i >> 4;
//         u32 lo = i & 0xF;
//         bool isUndefined = test_isArmInstructionUndefined(0xE6000010u | (hi << 5) | lo, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, BLUnconditionalSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x1000000; i++)
//     {
//         bool isUndefined = test_isArmInstructionUndefined(0xFF000000u | i, &inContext, &outContext);
//         EXPECT_THAT(isUndefined, Eq(true));
//     }
// }

// TEST(UndefinedInstructionTests, BLConditionalSubstituteIsUndefined)
// {
//     // Arrange
//     context_t inContext = gRandomContext;
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x1000000; i++)
//     {
//         if (((i >> 23) & 1) != ((i >> 22) & 1))
//         {
//             u32 hi = (i >> 21) & 3;
//             u32 mid = (i >> 20) & 1;
//             u32 lo = i & 0xFFF;
//             bool isUndefined = test_isArmInstructionUndefined(0xEC100000u | (hi << 23) | (mid << 21) | lo, &inContext, &outContext);
//             EXPECT_THAT(isUndefined, Eq(true));
//         }
//         else
//         {
//             u32 hi = (i >> 4) & 0x7FFFF;
//             u32 lo = i & 0xF;
//             bool isUndefined = test_isArmInstructionUndefined(0xEE000000u | (hi << 5) | lo, &inContext, &outContext);
//             EXPECT_THAT(isUndefined, Eq(true));
//         }
//     }
// }

// TEST(UndefinedInstructionTests, ThumbTest)
// {
//     // Arrange
//     context_t outContext;

//     // Act + Assert
//     for (u32 i = 0; i < 0x10000; i++)
//     {
//         if ((i & 0xFF00) == 0x4700)
//         {
//             // bx
//             continue;
//         }
//         if ((i & 0xFF00) == 0x4400 && (i & 0x87) == 0x87)
//         {
//             // add pc
//             continue;
//         }
//         if ((i & 0xFF00) == 0x4600 && (i & 0x87) == 0x87)
//         {
//             // mov pc
//             continue;
//         }
//         if ((i & 0xFF00) == 0xBD00)
//         {
//             // pop pc
//             continue;
//         }
//         if ((i & 0xF000) == 0xD000 && (i & 0x0F00) != 0x0E00)
//         {
//             // bcond and swi
//             continue;
//         }
//         if ((i & 0xF800) == 0xE000)
//         {
//             // b
//             continue;
//         }
//         if ((i & 0xF800) == 0xF800)
//         {
//             // bl
//             continue;
//         }
//         if (((i & 0xF800) == 0xE800) && (i & 1) == 0)
//         {
//             // blx
//             continue;
//         }
//         if ((i & 0xFF00) == 0xBE00)
//         {
//             // bkpt
//             continue;
//         }
//         context_t inContext { };
//         bool isUndefined = test_isThumbInstructionUndefined(i, &inContext, &outContext);
//         if (isUndefined)
//             LOG_DEBUG("%x\n", i);
//     }
// }
