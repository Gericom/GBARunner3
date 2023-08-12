#include "common.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../ContextTest.h"
#include "VirtualMachine/VMDtcm.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

static const u32 oldRegs[7] =
{
    0xAA08AA08, 0xAA09AA09, 0xAA0AAA0A,
    0xAA0BAA0B, 0xAA0CAA0C, 0xAA0DAA0D, 0xAA0EAA0E
};

static const u32 newRegs[7] =
{
    0x55085508, 0x55095509, 0x550A550A,
    0x550B550B, 0x550C550C, 0x550D550D, 0x550E550E
};

extern "C" void VMModeTests_SwitchToMode();

class VMModeTestsSpLr : public ::testing::TestWithParam<std::tuple<int, int>> { };

TEST_P(VMModeTestsSpLr, CorrectSpAndLr)
{
    // Arrange
    int oldMode = std::get<0>(GetParam());
    int newMode = std::get<1>(GetParam());
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(oldRegsOut[5], Eq(oldRegs[5]));
    EXPECT_THAT(oldRegsOut[6], Eq(oldRegs[6]));
    EXPECT_THAT(newRegsOut[5], Eq(newRegs[5]));
    EXPECT_THAT(newRegsOut[6], Eq(newRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsSpLr, Values(
    // fiq -> 
    std::make_tuple(0x11, 0x12),
    std::make_tuple(0x11, 0x13),
    std::make_tuple(0x11, 0x17),
    std::make_tuple(0x11, 0x1B),
    std::make_tuple(0x11, 0x1F),
    // irq ->
    std::make_tuple(0x12, 0x11),
    std::make_tuple(0x12, 0x13),
    std::make_tuple(0x12, 0x17),
    std::make_tuple(0x12, 0x1B),
    std::make_tuple(0x12, 0x1F),
    // svc ->
    std::make_tuple(0x13, 0x11),
    std::make_tuple(0x13, 0x12),
    std::make_tuple(0x13, 0x17),
    std::make_tuple(0x13, 0x1B),
    std::make_tuple(0x13, 0x1F),
    // abt ->
    std::make_tuple(0x17, 0x11),
    std::make_tuple(0x17, 0x12),
    std::make_tuple(0x17, 0x13),
    std::make_tuple(0x17, 0x1B),
    std::make_tuple(0x17, 0x1F),
    // und ->
    std::make_tuple(0x1B, 0x11),
    std::make_tuple(0x1B, 0x12),
    std::make_tuple(0x1B, 0x13),
    std::make_tuple(0x1B, 0x17),
    std::make_tuple(0x1B, 0x1F),
    // sys ->
    std::make_tuple(0x1F, 0x11),
    std::make_tuple(0x1F, 0x12),
    std::make_tuple(0x1F, 0x13),
    std::make_tuple(0x1F, 0x17),
    std::make_tuple(0x1F, 0x1B)));

class VMModeTestsIrq : public ::testing::TestWithParam<int> { };

TEST_P(VMModeTestsIrq, FromIrqStoresIrqRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x12;
    int newMode = GetParam();
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_irq[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_irq[1], Eq(oldRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsIrq,
    Values(0x13, 0x17, 0x1B, 0x1F));

TEST(VMModeTests, FromIrqToFiqStoresIrqRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x12;
    int newMode = 0x11;
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[0], Eq(oldRegs[0]));
    EXPECT_THAT(vm_regs_sys[1], Eq(oldRegs[1]));
    EXPECT_THAT(vm_regs_sys[2], Eq(oldRegs[2]));
    EXPECT_THAT(vm_regs_sys[3], Eq(oldRegs[3]));
    EXPECT_THAT(vm_regs_sys[4], Eq(oldRegs[4]));
    EXPECT_THAT(vm_regs_irq[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_irq[1], Eq(oldRegs[6]));
}

class VMModeTestsSvc : public ::testing::TestWithParam<int> { };

TEST_P(VMModeTestsSvc, FromSvcStoresSvcRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x13;
    int newMode = GetParam();
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_svc[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_svc[1], Eq(oldRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsSvc,
    Values(0x12, 0x17, 0x1B, 0x1F));

TEST(VMModeTests, FromSvcToFiqStoresSvcRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x13;
    int newMode = 0x11;
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[0], Eq(oldRegs[0]));
    EXPECT_THAT(vm_regs_sys[1], Eq(oldRegs[1]));
    EXPECT_THAT(vm_regs_sys[2], Eq(oldRegs[2]));
    EXPECT_THAT(vm_regs_sys[3], Eq(oldRegs[3]));
    EXPECT_THAT(vm_regs_sys[4], Eq(oldRegs[4]));
    EXPECT_THAT(vm_regs_svc[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_svc[1], Eq(oldRegs[6]));
}

class VMModeTestsAbt : public ::testing::TestWithParam<int> { };

TEST_P(VMModeTestsAbt, FromAbtStoresAbtRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x17;
    int newMode = GetParam();
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_abt[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_abt[1], Eq(oldRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsAbt,
    Values(0x12, 0x13, 0x1B, 0x1F));

TEST(VMModeTests, FromAbtToFiqStoresAbtRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x17;
    int newMode = 0x11;
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[0], Eq(oldRegs[0]));
    EXPECT_THAT(vm_regs_sys[1], Eq(oldRegs[1]));
    EXPECT_THAT(vm_regs_sys[2], Eq(oldRegs[2]));
    EXPECT_THAT(vm_regs_sys[3], Eq(oldRegs[3]));
    EXPECT_THAT(vm_regs_sys[4], Eq(oldRegs[4]));
    EXPECT_THAT(vm_regs_abt[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_abt[1], Eq(oldRegs[6]));
}

class VMModeTestsUnd : public ::testing::TestWithParam<int> { };

TEST_P(VMModeTestsUnd, FromUndStoresUndRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x1B;
    int newMode = GetParam();
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_und[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_und[1], Eq(oldRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsUnd,
    Values(0x12, 0x13, 0x17, 0x1F));

TEST(VMModeTests, FromUndToFiqStoresUndRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x1B;
    int newMode = 0x11;
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[0], Eq(oldRegs[0]));
    EXPECT_THAT(vm_regs_sys[1], Eq(oldRegs[1]));
    EXPECT_THAT(vm_regs_sys[2], Eq(oldRegs[2]));
    EXPECT_THAT(vm_regs_sys[3], Eq(oldRegs[3]));
    EXPECT_THAT(vm_regs_sys[4], Eq(oldRegs[4]));
    EXPECT_THAT(vm_regs_und[0], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_und[1], Eq(oldRegs[6]));
}

class VMModeTestsSys : public ::testing::TestWithParam<int> { };

TEST_P(VMModeTestsSys, FromSysStoresSysRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x1F;
    int newMode = GetParam();
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[5], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_sys[6], Eq(oldRegs[6]));
}

INSTANTIATE_TEST_SUITE_P(, VMModeTestsSys,
    Values(0x12, 0x13, 0x17, 0x1B));

TEST(VMModeTests, FromSysToFiqStoresSysRegsInCorrectDtcmVariable)
{
    // Arrange
    int oldMode = 0x1F;
    int newMode = 0x11;
    VirtualMachine vm { (void*)VMModeTests_SwitchToMode, nullptr, nullptr };
    u32 oldRegsOut[7];
    u32 newRegsOut[7];
    context_t context = gRandomContext;
    context.r[0] = oldMode;
    context.r[1] = newMode;
    context.r[2] = (u32)oldRegs;
    context.r[3] = (u32)oldRegsOut;
    context.r[4] = (u32)newRegs;
    context.r[5] = (u32)newRegsOut;

    // Act
    vm.Run(&context);

    // Assert
    EXPECT_THAT(vm_regs_sys[0], Eq(oldRegs[0]));
    EXPECT_THAT(vm_regs_sys[1], Eq(oldRegs[1]));
    EXPECT_THAT(vm_regs_sys[2], Eq(oldRegs[2]));
    EXPECT_THAT(vm_regs_sys[3], Eq(oldRegs[3]));
    EXPECT_THAT(vm_regs_sys[4], Eq(oldRegs[4]));
    EXPECT_THAT(vm_regs_sys[5], Eq(oldRegs[5]));
    EXPECT_THAT(vm_regs_sys[6], Eq(oldRegs[6]));
}