#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

static u32 vmTestFunc(u32 x)
{
    return x + 0xAABB;
}

TEST(VMRunTests, RunsStartFunctionAndReturns)
{
    // Arrange
    VirtualMachine vm { (void*)vmTestFunc, nullptr, nullptr };
    u32 x = 0x1234;
    u32 stack[32];
    context_t context { x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (u32)stack, 0 };

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(0x1234 + 0xAABB));
}

extern "C" u32 VMRunTests_GetVMCpsr(void);

TEST(VMRunTests, StartsExecutionInSvcMode)
{
    // Arrange
    VirtualMachine vm { (void*)VMRunTests_GetVMCpsr, nullptr, nullptr };
    context_t context { };

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result & 0x1F, Eq(0x13));
}

TEST(VMRunTests, StartsExecutionWithIrqsDisabled)
{
    // Arrange
    VirtualMachine vm { (void*)VMRunTests_GetVMCpsr, nullptr, nullptr };
    context_t context { };

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result & 0x80, Eq(0x80));
}

TEST(VMRunTests, StartsExecutionWithFiqsDisabled)
{
    // Arrange
    VirtualMachine vm { (void*)VMRunTests_GetVMCpsr, nullptr, nullptr };
    context_t context { };

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result & 0x40, Eq(0x40));
}
