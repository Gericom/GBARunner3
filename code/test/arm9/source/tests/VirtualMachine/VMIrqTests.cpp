#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ContextTest.h"
#include "VirtualMachine/VMDtcm.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

extern "C" bool VMIrqTests_IrqsEnabledWithPendingIrqJumpsToIrqVector(void);
extern "C" void VMIrqTests_IrqVector(void);

TEST(VMIrqTests, IrqsEnabledWithPendingIrqJumpsToIrqVector)
{
    // Arrange
    VirtualMachine vm
    {
        (void*)VMIrqTests_IrqsEnabledWithPendingIrqJumpsToIrqVector,
        nullptr,
        (void*)VMIrqTests_IrqVector
    };
    context_t context = gRandomContext;

    // Act
    bool result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(true));
}
