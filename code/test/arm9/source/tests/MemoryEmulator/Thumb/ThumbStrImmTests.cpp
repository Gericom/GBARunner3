#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "../../ContextTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrImmOffset : public testing::TestWithParam<int> { };

TEST_P(ThumbStrImmOffset, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;
    const int offset = GetParam();

    // Act
    test_runThumbInstruction(THUMB_STR_IMM(THUMB_R0, THUMB_R1, offset), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050 + offset));
    EXPECT_THAT(memu_stubStoredValue32, Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrImmOffset, Range(0, 128, 4), PrintToStringParamName());

class ThumbStrImmRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STR_IMM(rd, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue32, Eq(inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrImmRd, Range(0, 8), PrintToStringParamName());

class ThumbStrImmRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrImmRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STR_IMM(THUMB_R0, rn, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue32, Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrImmRn, Range(0, 8), PrintToStringParamName());

class ThumbStrImmAlignment : public testing::TestWithParam<int> { };

TEST_P(ThumbStrImmAlignment, AddressForceAligned)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    const int alignOffset = GetParam();
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050 + alignOffset;

    // Act
    test_runThumbInstruction(THUMB_STR_IMM(THUMB_R0, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue32, Eq(0xAABBCCDD));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrImmAlignment, Values(1, 2, 3), PrintToStringParamName());
