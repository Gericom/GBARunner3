#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrhImmOffset : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhImmOffset, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    inContext.r[0] = 0xAABB;
    inContext.r[1] = 0x10000050;
    const int offset = GetParam();

    // Act
    test_runThumbInstruction(THUMB_STRH_IMM(THUMB_R0, THUMB_R1, offset), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050 + offset));
    EXPECT_THAT(memu_stubStoredValue16, Eq(0xAABB));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhImmOffset, Range(0, 64, 2), PrintToStringParamName());

class ThumbStrhImmRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhImmRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABB;
    inContext.r[1] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRH_IMM(rd, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue16, Eq((u16)inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhImmRd, Range(0, 8), PrintToStringParamName());

class ThumbStrhImmRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhImmRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABB;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRH_IMM(THUMB_R0, rn, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue16, Eq((u16)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhImmRn, Range(0, 8), PrintToStringParamName());

TEST(ThumbStrhImm, AddressForceAligned)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    inContext.r[0] = 0xAABB;
    inContext.r[1] = 0x10000051;

    // Act
    test_runThumbInstruction(THUMB_STRH_IMM(THUMB_R0, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue16, Eq(0xAABB));
}

TEST(ThumbStrhImm, ValueMasked)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRH_IMM(THUMB_R0, THUMB_R1, 0), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValue16, Eq(0xCCDD));
}
