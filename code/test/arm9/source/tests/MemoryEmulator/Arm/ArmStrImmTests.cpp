#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"

using namespace ::testing;

class ArmStrImmOffset : public testing::TestWithParam<int> { };

TEST(ArmStrImm, CorrectOffsetComputed)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5810000u | 0xABC, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValue32, Eq(0xAABBCCDD));
}

TEST(ArmStrImmPreWriteBack, PerformsWriteBack)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddress32 = 0;
    memu_stubStoredValue32 = 0;
    inContext.r[0] = 0xAABBCCDD;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE5A10000u | 0xABC, &inContext, &outContext);

    // Assert
    // EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(outContext.r[1], Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredAddress32, Eq(0x10000050 + 0xABC));
    EXPECT_THAT(memu_stubStoredValue32, Eq(0xAABBCCDD));
}
