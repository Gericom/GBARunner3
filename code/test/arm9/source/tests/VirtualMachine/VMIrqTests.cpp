#include "common.h"
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/gfx/gfx3d.h>
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
    REG_GXSTAT = 0;
    rtos_ackIrqMask(RTOS_IRQ_GX_FIFO);
    rtos_setIrqMask(RTOS_IRQ_GX_FIFO);
    REG_IME = 1;

    // Act
    bool result = vm.Run(&context);

    // Assert
    EXPECT_THAT(result, Eq(true));

    REG_IME = 0;
    rtos_setIrqMask(0);
}
