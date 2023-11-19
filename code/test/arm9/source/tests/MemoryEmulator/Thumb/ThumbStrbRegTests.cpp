#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "../../ContextTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrbRegRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbRegRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xA5;
    inContext.r[1] = 0x0F000050;
    inContext.r[2] = 4;

    // Act
    test_runThumbInstruction(THUMB_STRB_REG(rd, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x0F000054));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbRegRd, Range(0, 8), PrintToStringParamName());

class ThumbStrbRegRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbRegRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xA5;
    inContext.r[2] = 4;
    inContext.r[rn] = 0x00231050;

    // Act
    test_runThumbInstruction(THUMB_STRB_REG(THUMB_R0, rn, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x00231050 + inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbRegRn, Range(0, 8), PrintToStringParamName());

class ThumbStrbRegRm : public testing::TestWithParam<int> { };

TEST_P(ThumbStrbRegRm, UsesCorrectRmValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    const int rm = GetParam();
    inContext.r[0] = 0xA5;
    inContext.r[1] = 4;
    inContext.r[rm] = 0x00231050;

    // Act
    test_runThumbInstruction(THUMB_STRB_REG(THUMB_R0, THUMB_R1, rm), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x00231050 + inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValue8, Eq((u8)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrbRegRm, Range(0, 8), PrintToStringParamName());

TEST(ThumbStrbReg, ValueMasked)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress8 = 0;
    memu_stubStoredValue8 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x0F000050;
    inContext.r[2] = 4;

    // Act
    test_runThumbInstruction(THUMB_STRB_REG(THUMB_R0, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress8, Eq(0x0F000054));
    EXPECT_THAT(memu_stubStoredValue8, Eq(0xDD));
}
