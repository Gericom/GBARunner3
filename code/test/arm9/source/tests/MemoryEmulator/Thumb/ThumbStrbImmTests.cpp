#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "../../ContextTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrbImmOffset : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbImmOffset, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    inContext.r[0] = 0xA5;
    inContext.r[1] = 0x10000050;
    const int offset = GetParam();

    // Act
    test_runThumbInstruction(THUMB_STRB_IMM(THUMB_R0, THUMB_R1, offset), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050 + offset));
    EXPECT_THAT(memu_stubStoredValue8, Eq(0xA5));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbImmOffset, Range(0, 32), PrintToStringParamName());

class ThumbStrbImmRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xA5;
    inContext.r[1] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRB_IMM(rd, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbImmRd, Range(0, 8), PrintToStringParamName());

class ThumbStrbImmRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbImmRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xA5;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRB_IMM(THUMB_R0, rn, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbImmRn, Range(0, 8), PrintToStringParamName());

TEST(ThumbStrbImm, ValueMasked)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRB_IMM(THUMB_R0, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue8, Eq(0xDD));
}
