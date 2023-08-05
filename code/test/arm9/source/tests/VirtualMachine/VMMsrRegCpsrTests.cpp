#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ContextTest.h"
#include "VirtualMachine/VMDtcm.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

extern "C" u32 VMMsrTests_SetVMCpsrFlagsR0(void);
extern "C" u32 VMMsrTests_SetVMCpsrFlagsR0VerifyHwCpsr(void);
extern "C" u32 VMMsrTests_SetVMCpsrControlR0(void);

TEST(VMMsrRegCpsr, AppliesFlagsMask)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMCpsrFlagsR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xFFFFFFFF;

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(0xF00000D3));
}

TEST(VMMsrRegCpsr, FlagsStoredInHwCpsr)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMCpsrFlagsR0VerifyHwCpsr, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xAFFFFFFF;

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result & 0xF0000000, Eq(0xA0000000));
}

TEST(VMMsrRegCpsr, AppliesControlMask)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMCpsrControlR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xFFFFFFFF;

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(0x000000DF));
}
