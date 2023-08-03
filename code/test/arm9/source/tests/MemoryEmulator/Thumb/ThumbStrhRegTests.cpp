#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ThumbTest.h"
#include "../../ContextTest.h"
#include "ThumbInstructions.h"

using namespace ::testing;

class ThumbStrhRegRd : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhRegRd, UsesCorrectRdValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    const int rd = GetParam();
    inContext.r[rd] = 0xAABB;
    inContext.r[1] = 0x10000050;
    inContext.r[2] = 4;

    // Act
    test_runThumbInstruction(THUMB_STRH_REG(rd, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredValue16, Eq((u16)inContext.r[rd]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhRegRd, Range(0, 8), PrintToStringParamName());

class ThumbStrhRegRn : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhRegRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    const int rn = GetParam();
    inContext.r[0] = 0xAABB;
    inContext.r[2] = 4;
    inContext.r[rn] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRH_REG(THUMB_R0, rn, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050 + inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValue16, Eq((u16)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhRegRn, Range(0, 8), PrintToStringParamName());

class ThumbStrhRegRm : public testing::TestWithParam<int> { };

TEST_P(ThumbStrhRegRm, UsesCorrectRmValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    const int rm = GetParam();
    inContext.r[0] = 0xAABB;
    inContext.r[1] = 4;
    inContext.r[rm] = 0x10000050;

    // Act
    test_runThumbInstruction(THUMB_STRH_REG(THUMB_R0, THUMB_R1, rm), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000050 + inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValue16, Eq((u16)inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ThumbStrhRegRm, Range(0, 8), PrintToStringParamName());

TEST(ThumbStrhReg, AddressForceAligned)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    inContext.r[0] = 0xAABB;
    inContext.r[1] = 0x10000052;
    inContext.r[2] = 1;

    // Act
    test_runThumbInstruction(THUMB_STRH_REG(THUMB_R0, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000052));
    EXPECT_THAT(memu_stubStoredValue16, Eq(0xAABB));
}

TEST(ThumbStrhReg, ValueMasked)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress16 = 0;
    memu_stubStoredValue16 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;
    inContext.r[2] = 4;

    // Act
    test_runThumbInstruction(THUMB_STRH_REG(THUMB_R0, THUMB_R1, THUMB_R2), &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress16, Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredValue16, Eq(0xCCDD));
}
