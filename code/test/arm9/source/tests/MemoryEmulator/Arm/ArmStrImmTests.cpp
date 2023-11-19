#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"
#include "../../ContextTest.h"

using namespace ::testing;

class ArmStrImmRd : public testing::TestWithParam<int> { };

TEST_P(ArmStrImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABBCCDD;
    inContext.r[1] = 0x0F000050;

    // Act
    test_runArmInstruction(0xE5810000u | (rd << 12), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x0F000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrImmRd, Range(0, 15), PrintToStringParamName());

class ArmStrImmRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrImmRn, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x0F000050;

    // Act
    test_runArmInstruction(0xE5800000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x0F000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrImmRn, Range(0, 15), PrintToStringParamName());

class ArmStrImmPreWritebackRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrImmPreWritebackRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x0F000050;

    // Act
    test_runArmInstruction(0xE5A00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x0F000050 + 0xABC));
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x0F000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrImmPreWritebackRn, Range(0, 15), PrintToStringParamName());

class ArmStrImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x0F000050;

    // Act
    test_runArmInstruction(0xE4800000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x0F000050 + 0xABC));
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x0F000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrImmPostRn, Range(0, 15), PrintToStringParamName());

class ArmStrtImmPostRn : public testing::TestWithParam<int> { };

TEST_P(ArmStrtImmPostRn, PerformsWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x0F000050;

    // Act
    test_runArmInstruction(0xE4A00000u | 0xABC | (rn << 16), &inContext, &outContext);

    // Assert
    EXPECT_THAT(outContext.r[rn], Eq(0x0F000050 + 0xABC));
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x0F000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStrtImmPostRn, Range(0, 15), PrintToStringParamName());
