#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ContextTest.h"
#include "VirtualMachine/VMDtcm.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

extern "C" u32 VMAluSPCImmTests_SUBS_PC_R0_0(u32 newCpsr);

TEST(VMAluSPCImm, CopiesSpsrToCpsr)
{
    // Arrange
    VirtualMachine vm { (void*)VMAluSPCImmTests_SUBS_PC_R0_0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xA00000DF;

    // Act
    u32 result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(0xA00000DF));
}
