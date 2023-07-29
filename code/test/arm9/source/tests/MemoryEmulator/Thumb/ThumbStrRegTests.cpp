#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrRegRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrRegRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;
    inContext.r[2] = 4;

    // Act
    test_runThumbInstruction(THUMB_STR_REG(rd, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredValue32, Eq(inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrRegRd, Range(0, 8), PrintToStringParamName());

class ThumbStrRegRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrRegRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[2] = 4;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STR_REG(THUMB_R0, rn, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050 + inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValue32, Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrRegRn, Range(0, 8), PrintToStringParamName());

class ThumbStrRegRm : public testing::TestWithParam<int> { };

TEST_P(ThumbStrRegRm, UsesCorrectRmValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int rm = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 4;
    inContext.r[rm] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STR_REG(THUMB_R0, THUMB_R1, rm), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050 + inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValue32, Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrRegRm, Range(0, 8), PrintToStringParamName());

class ThumbStrRegAlignment : public testing::TestWithParam<int> { };

TEST_P(ThumbStrRegAlignment, AddressForceAligned)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int alignOffset = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;
    inContext.r[2] = alignOffset;

    // Act
    test_runThumbInstruction(THUMB_STR_REG(THUMB_R0, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue32, Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrRegAlignment, Values(1, 2, 3), PrintToStringParamName());
