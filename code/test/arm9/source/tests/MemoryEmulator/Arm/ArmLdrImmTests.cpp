#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"
#include "../../ContextTest.h"

using namespace ::testing;

class ArmLdrImmRd : public testing::TestWithParam<int> { };

TEST_P(ArmLdrImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress32 = 0;
    memu_stubLoadValue32 = 0xAABBCCDD;
    const int rd = GetParam();
    inContext.r[0] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5900000u | (rd << 12), &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubLoadedAddress32, Eq(0x10000050));
    EXPECT_THAT(outContext.r[rd], Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrImmRd, Range(0, 15), PrintToStringParamName());

class ArmLdrImmRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrImmRn, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress32 = 0;
    memu_stubLoadValue32 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5900000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubLoadedAddress32, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(outContext.r[0], Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrImmRn, Range(0, 15), PrintToStringParamName());

class ArmLdrImmPreWritebackRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrImmPreWritebackRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress32 = 0;
    memu_stubLoadValue32 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5B00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress32, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(outContext.r[0], Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrImmPreWritebackRn, Range(0, 15), PrintToStringParamName());

class ArmLdrImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress32 = 0;
    memu_stubLoadValue32 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4900000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress32, Eq(0x10000050));
    EXPECT_THAT(outContext.r[0], Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrImmPostRn, Range(0, 15), PrintToStringParamName());

class ArmLdrtImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmLdrtImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubLoadedAddress32 = 0;
    memu_stubLoadValue32 = 0xAABBCCDD;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE4B00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    if (rn != 0)
    {
        EXPECT_THAT(outContext.r[rn], Eq(0x10000050 + 0xABC));
    }
    EXPECT_THAT(memu_stubLoadedAddress32, Eq(0x10000050));
    EXPECT_THAT(outContext.r[0], Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ArmLdrtImmPostRn, Range(0, 15), PrintToStringParamName());
