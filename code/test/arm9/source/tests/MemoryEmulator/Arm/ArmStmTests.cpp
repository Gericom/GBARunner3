#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MemoryStub.h"
#include "../../ArmTest.h"
#include "../../ContextTest.h"

using namespace ::testing;

class ArmStmiaRn : public testing::TestWithParam<int> { };

TEST_P(ArmStmiaRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8800000u | (rn << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStmiaRn, Range(0, 15), PrintToStringParamName());

class ArmStmiaWritebackRn : public testing::TestWithParam<int> { };

TEST_P(ArmStmiaWritebackRn, WritesBackToCorrectRn)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8A00000u | (rn << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(outContext.r[rn], Eq(0x10000054));
}

INSTANTIATE_TEST_SUITE_P(, ArmStmiaWritebackRn, Range(0, 15), PrintToStringParamName());

TEST(ArmStmiaTests, StoresNewRnForWritebackWithRnInRlistNotFirstReg)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8A00000u | (1 << 16) | 7, &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubStore32Count, Eq(3));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000058));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(0x1000005C));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[2]));
    EXPECT_THAT(outContext.r[1], Eq(0x1000005C));
}

TEST(ArmStmiaTests, ForceAligns)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000053;

    // Act
    test_runArmInstruction(0xE8800000u | (1 << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

TEST(ArmStmiaTests, ForceAlignNotAppliedToWriteback)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000053;

    // Act
    test_runArmInstruction(0xE8A00000u | (1 << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(outContext.r[1], Eq(0x10000057));
}

TEST(ArmStmiaTests, StoresAllRegisters)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8800000u | (1 << 16) | 0xFFFF, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(16));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000058));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x1000005C));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000060));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000064));
    EXPECT_THAT(memu_stubStoredAddresses32[6], Eq(0x10000068));
    EXPECT_THAT(memu_stubStoredAddresses32[7], Eq(0x1000006C));
    EXPECT_THAT(memu_stubStoredAddresses32[8], Eq(0x10000070));
    EXPECT_THAT(memu_stubStoredAddresses32[9], Eq(0x10000074));
    EXPECT_THAT(memu_stubStoredAddresses32[10], Eq(0x10000078));
    EXPECT_THAT(memu_stubStoredAddresses32[11], Eq(0x1000007C));
    EXPECT_THAT(memu_stubStoredAddresses32[12], Eq(0x10000080));
    EXPECT_THAT(memu_stubStoredAddresses32[13], Eq(0x10000084));
    EXPECT_THAT(memu_stubStoredAddresses32[14], Eq(0x10000088));
    EXPECT_THAT(memu_stubStoredAddresses32[15], Eq(0x1000008C));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[3]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[4]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[5]));
    EXPECT_THAT(memu_stubStoredValues32[6], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[7], Eq(inContext.r[7]));
    EXPECT_THAT(memu_stubStoredValues32[8], Eq(inContext.r[8]));
    EXPECT_THAT(memu_stubStoredValues32[9], Eq(inContext.r[9]));
    EXPECT_THAT(memu_stubStoredValues32[10], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[11], Eq(inContext.r[11]));
    EXPECT_THAT(memu_stubStoredValues32[12], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[13], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[15], Eq((u32)test_runArmInstruction_instruction + 12));
}

TEST(ArmStmiaTests, StoresWithGaps)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8800000u | (1 << 16) | 0x7442, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(6));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000058));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x1000005C));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000060));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000064));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[14]));
}

class ArmStmibRn : public testing::TestWithParam<int> { };

TEST_P(ArmStmibRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9800000u | (rn << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStmibRn, Range(0, 15), PrintToStringParamName());

TEST(ArmStmibTests, ForceAligns)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000053;

    // Act
    test_runArmInstruction(0xE9800000u | (1 << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

TEST(ArmStmibTests, StoresAllRegisters)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9800000u | (1 << 16) | 0xFFFF, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(16));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000058));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x1000005C));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x10000060));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000064));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000068));
    EXPECT_THAT(memu_stubStoredAddresses32[6], Eq(0x1000006C));
    EXPECT_THAT(memu_stubStoredAddresses32[7], Eq(0x10000070));
    EXPECT_THAT(memu_stubStoredAddresses32[8], Eq(0x10000074));
    EXPECT_THAT(memu_stubStoredAddresses32[9], Eq(0x10000078));
    EXPECT_THAT(memu_stubStoredAddresses32[10], Eq(0x1000007C));
    EXPECT_THAT(memu_stubStoredAddresses32[11], Eq(0x10000080));
    EXPECT_THAT(memu_stubStoredAddresses32[12], Eq(0x10000084));
    EXPECT_THAT(memu_stubStoredAddresses32[13], Eq(0x10000088));
    EXPECT_THAT(memu_stubStoredAddresses32[14], Eq(0x1000008C));
    EXPECT_THAT(memu_stubStoredAddresses32[15], Eq(0x10000090));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[3]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[4]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[5]));
    EXPECT_THAT(memu_stubStoredValues32[6], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[7], Eq(inContext.r[7]));
    EXPECT_THAT(memu_stubStoredValues32[8], Eq(inContext.r[8]));
    EXPECT_THAT(memu_stubStoredValues32[9], Eq(inContext.r[9]));
    EXPECT_THAT(memu_stubStoredValues32[10], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[11], Eq(inContext.r[11]));
    EXPECT_THAT(memu_stubStoredValues32[12], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[13], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[15], Eq((u32)test_runArmInstruction_instruction + 12));
}

TEST(ArmStmibTests, StoresWithGaps)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9800000u | (1 << 16) | 0x7442, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(6));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000054));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000058));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x1000005C));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x10000060));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000064));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000068));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[14]));
}

class ArmStmdaRn : public testing::TestWithParam<int> { };

TEST_P(ArmStmdaRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8000000u | (rn << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStmdaRn, Range(0, 15), PrintToStringParamName());

TEST(ArmStmdaTests, ForceAligns)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000053;

    // Act
    test_runArmInstruction(0xE8000000u | (1 << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000050));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

TEST(ArmStmdaTests, StoresAllRegisters)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8000000u | (1 << 16) | 0xFFFF, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(16));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000014));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000018));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x1000001C));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x10000020));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000024));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000028));
    EXPECT_THAT(memu_stubStoredAddresses32[6], Eq(0x1000002C));
    EXPECT_THAT(memu_stubStoredAddresses32[7], Eq(0x10000030));
    EXPECT_THAT(memu_stubStoredAddresses32[8], Eq(0x10000034));
    EXPECT_THAT(memu_stubStoredAddresses32[9], Eq(0x10000038));
    EXPECT_THAT(memu_stubStoredAddresses32[10], Eq(0x1000003C));
    EXPECT_THAT(memu_stubStoredAddresses32[11], Eq(0x10000040));
    EXPECT_THAT(memu_stubStoredAddresses32[12], Eq(0x10000044));
    EXPECT_THAT(memu_stubStoredAddresses32[13], Eq(0x10000048));
    EXPECT_THAT(memu_stubStoredAddresses32[14], Eq(0x1000004C));
    EXPECT_THAT(memu_stubStoredAddresses32[15], Eq(0x10000050));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[3]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[4]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[5]));
    EXPECT_THAT(memu_stubStoredValues32[6], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[7], Eq(inContext.r[7]));
    EXPECT_THAT(memu_stubStoredValues32[8], Eq(inContext.r[8]));
    EXPECT_THAT(memu_stubStoredValues32[9], Eq(inContext.r[9]));
    EXPECT_THAT(memu_stubStoredValues32[10], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[11], Eq(inContext.r[11]));
    EXPECT_THAT(memu_stubStoredValues32[12], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[13], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[15], Eq((u32)test_runArmInstruction_instruction + 12));
}

TEST(ArmStmdaTests, StoresWithGaps)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE8000000u | (1 << 16) | 0x7442, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(6));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x1000003C));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000040));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000044));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x10000048));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x1000004C));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000050));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[14]));
}

class ArmStmdbRn : public testing::TestWithParam<int> { };

TEST_P(ArmStmdbRn, UsesCorrectRnValue)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    const int rn = GetParam();
    inContext.r[rn] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9000000u | (rn << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x1000004C));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

INSTANTIATE_TEST_SUITE_P(, ArmStmdbRn, Range(0, 15), PrintToStringParamName());

TEST(ArmStmdbTests, ForceAligns)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memu_stubStoredAddresses32[0] = 0;
    memu_stubStoredValues32[0] = 0;
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000053;

    // Act
    test_runArmInstruction(0xE9000000u | (1 << 16) | 1, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(1));
    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x1000004C));
    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
}

TEST(ArmStmdbTests, StoresAllRegisters)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9000000u | (1 << 16) | 0xFFFF, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(16));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000010));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x10000014));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000018));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x1000001C));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000020));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x10000024));
    EXPECT_THAT(memu_stubStoredAddresses32[6], Eq(0x10000028));
    EXPECT_THAT(memu_stubStoredAddresses32[7], Eq(0x1000002C));
    EXPECT_THAT(memu_stubStoredAddresses32[8], Eq(0x10000030));
    EXPECT_THAT(memu_stubStoredAddresses32[9], Eq(0x10000034));
    EXPECT_THAT(memu_stubStoredAddresses32[10], Eq(0x10000038));
    EXPECT_THAT(memu_stubStoredAddresses32[11], Eq(0x1000003C));
    EXPECT_THAT(memu_stubStoredAddresses32[12], Eq(0x10000040));
    EXPECT_THAT(memu_stubStoredAddresses32[13], Eq(0x10000044));
    EXPECT_THAT(memu_stubStoredAddresses32[14], Eq(0x10000048));
    EXPECT_THAT(memu_stubStoredAddresses32[15], Eq(0x1000004C));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[0]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[2]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[3]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[4]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[5]));
    EXPECT_THAT(memu_stubStoredValues32[6], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[7], Eq(inContext.r[7]));
    EXPECT_THAT(memu_stubStoredValues32[8], Eq(inContext.r[8]));
    EXPECT_THAT(memu_stubStoredValues32[9], Eq(inContext.r[9]));
    EXPECT_THAT(memu_stubStoredValues32[10], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[11], Eq(inContext.r[11]));
    EXPECT_THAT(memu_stubStoredValues32[12], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[13], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[14], Eq(inContext.r[14]));
    EXPECT_THAT(memu_stubStoredValues32[15], Eq((u32)test_runArmInstruction_instruction + 12));
}

TEST(ArmStmdbTests, StoresWithGaps)
{
    // Arrange
    context_t inContext = gRandomContext;
    context_t outContext;
    memset(memu_stubStoredAddresses32, 0, sizeof(memu_stubStoredAddresses32));
    memset(memu_stubStoredValues32, 0, sizeof(memu_stubStoredValues32));
    memu_stubStore32Count = 0;
    inContext.r[1] = 0x10000050;

    // Act
    test_runArmInstruction(0xE9000000u | (1 << 16) | 0x7442, &inContext, &outContext);

    // Assert
    EXPECT_CONTEXT_EQ(outContext, inContext);
    EXPECT_THAT(memu_stubStore32Count, Eq(6));

    EXPECT_THAT(memu_stubStoredAddresses32[0], Eq(0x10000038));
    EXPECT_THAT(memu_stubStoredAddresses32[1], Eq(0x1000003C));
    EXPECT_THAT(memu_stubStoredAddresses32[2], Eq(0x10000040));
    EXPECT_THAT(memu_stubStoredAddresses32[3], Eq(0x10000044));
    EXPECT_THAT(memu_stubStoredAddresses32[4], Eq(0x10000048));
    EXPECT_THAT(memu_stubStoredAddresses32[5], Eq(0x1000004C));

    EXPECT_THAT(memu_stubStoredValues32[0], Eq(inContext.r[1]));
    EXPECT_THAT(memu_stubStoredValues32[1], Eq(inContext.r[6]));
    EXPECT_THAT(memu_stubStoredValues32[2], Eq(inContext.r[10]));
    EXPECT_THAT(memu_stubStoredValues32[3], Eq(inContext.r[12]));
    EXPECT_THAT(memu_stubStoredValues32[4], Eq(inContext.r[13]));
    EXPECT_THAT(memu_stubStoredValues32[5], Eq(inContext.r[14]));
}