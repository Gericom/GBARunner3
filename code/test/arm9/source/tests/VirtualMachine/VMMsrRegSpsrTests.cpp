#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ContextTest.h"
#include "VirtualMachine/VMDtcm.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

extern "C" void VMMsrTests_SetVMSpsrR0(void);
extern "C" void VMMsrTests_SetVMSpsrR1(void);
extern "C" void VMMsrTests_SetVMSpsrR2(void);
extern "C" void VMMsrTests_SetVMSpsrR3(void);
extern "C" void VMMsrTests_SetVMSpsrR4(void);
extern "C" void VMMsrTests_SetVMSpsrR5(void);
extern "C" void VMMsrTests_SetVMSpsrR6(void);
extern "C" void VMMsrTests_SetVMSpsrR7(void);
extern "C" void VMMsrTests_SetVMSpsrR8(void);
extern "C" void VMMsrTests_SetVMSpsrR9(void);
extern "C" void VMMsrTests_SetVMSpsrR10(void);
extern "C" void VMMsrTests_SetVMSpsrR11(void);
extern "C" void VMMsrTests_SetVMSpsrR12(void);
extern "C" void VMMsrTests_SetVMSpsrR13(void);
extern "C" void VMMsrTests_SetVMSpsrFlagsR0(void);
extern "C" void VMMsrTests_SetVMSpsrControlR0(void);

static void* VMMsrTests_SetVMSpsrRm[15] =
{
    (void*)VMMsrTests_SetVMSpsrR0,
    (void*)VMMsrTests_SetVMSpsrR1,
    (void*)VMMsrTests_SetVMSpsrR2,
    (void*)VMMsrTests_SetVMSpsrR3,
    (void*)VMMsrTests_SetVMSpsrR4,
    (void*)VMMsrTests_SetVMSpsrR5,
    (void*)VMMsrTests_SetVMSpsrR6,
    (void*)VMMsrTests_SetVMSpsrR7,
    (void*)VMMsrTests_SetVMSpsrR8,
    (void*)VMMsrTests_SetVMSpsrR9,
    (void*)VMMsrTests_SetVMSpsrR10,
    (void*)VMMsrTests_SetVMSpsrR11,
    (void*)VMMsrTests_SetVMSpsrR12,
    (void*)VMMsrTests_SetVMSpsrR13
};

class VMMsrRegSpsrRm : public testing::TestWithParam<int> { };

TEST_P(VMMsrRegSpsrRm, UsesCorrectRmValue)
{
    // Arrange
    const int rm = GetParam();
    VirtualMachine vm { VMMsrTests_SetVMSpsrRm[rm], nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[rm] = 0xA0000097;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_spsr_svc, Eq(0xA0000097));
}

INSTANTIATE_TEST_SUITE_P(, VMMsrRegSpsrRm, Range(0, 14), PrintToStringParamName());

TEST(VMMsrRegSpsr, AppliesFixedZeros)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMSpsrR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xFFFFFFFF;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_spsr_svc, Eq(0xF00000FF));
}

TEST(VMMsrRegSpsr, AppliesFixedOne)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMSpsrR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_spsr_svc, Eq(0x00000010));
}

TEST(VMMsrRegSpsr, AppliesFlagsMask)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMSpsrFlagsR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xFFFFFFFF;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_spsr_svc, Eq(0xF0000000));
}

TEST(VMMsrRegSpsr, AppliesControlMask)
{
    // Arrange
    VirtualMachine vm { (void*)VMMsrTests_SetVMSpsrControlR0, nullptr, nullptr };
    context_t context = gRandomContext;
    context.r[0] = 0xFFFFFFFF;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_spsr_svc, Eq(0x000000FF));
}
