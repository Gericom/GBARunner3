#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"

using namespace ::testing;

class ArmStrbImmRd : public testing::TestWithParam<int> { };

TEST_P(ArmStrbImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5C10000u | (rd << 12), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrbImmRd, Range(0, 15), PrintToStringParamName());

class ArmStrbImmRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrbImmRn, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5C00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrbImmRn, Range(0, 15), PrintToStringParamName());

class ArmStrbImmPreWritebackRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrbImmPreWritebackRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5E00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrbImmPreWritebackRn, Range(0, 15), PrintToStringParamName());

class ArmStrbImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrbImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4C00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrbImmPostRn, Range(0, 15), PrintToStringParamName());

class ArmStrbtImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrbtImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4E00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrbtImmPostRn, Range(0, 15), PrintToStringParamName());
