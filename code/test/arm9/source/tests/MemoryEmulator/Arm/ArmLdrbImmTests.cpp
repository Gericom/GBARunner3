#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"

using namespace ::testing;

class ArmLdrbImmRd : public testing::TestWithParam<int> { };

TEST_P(ArmLdrbImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress8 = 0;
    memu_stubLoadValue8 = 0xAABBCCDD;
    const int rd = GetParam();
    inContext.r[0] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5D00000u | (rd << 12), &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubLoadedAddress8, Eq(0x10000050));
    EXPECT_THAT(outContext.r[rd], Eq(0xDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrbImmRd, Range(0, 15), PrintToStringParamName());

class ArmLdrbImmRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrbImmRn, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress8 = 0;
    memu_stubLoadValue8 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5D00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubLoadedAddress8, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(outContext.r[0], Eq(0xDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrbImmRn, Range(0, 15), PrintToStringParamName());

class ArmLdrbImmPreWritebackRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrbImmPreWritebackRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress8 = 0;
    memu_stubLoadValue8 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5F00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress8, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(outContext.r[0], Eq(0xDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrbImmPreWritebackRn, Range(0, 15), PrintToStringParamName());

class ArmLdrbImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrbImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress8 = 0;
    memu_stubLoadValue8 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4D00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress8, Eq(0x10000050));
    EXPECT_THAT(outContext.r[0], Eq(0xDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrbImmPostRn, Range(0, 15), PrintToStringParamName());

class ArmLdrbtImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrbtImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress8 = 0;
    memu_stubLoadValue8 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4F00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress8, Eq(0x10000050));
    EXPECT_THAT(outContext.r[0], Eq(0xDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrbtImmPostRn, Range(0, 15), PrintToStringParamName());
