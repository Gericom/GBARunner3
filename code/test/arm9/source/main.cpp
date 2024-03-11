#include "common.h"
#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "core/Environment.h"
#include "logger/PlainLogger.h"
#include "logger/NocashOutputStream.h"
#include "logger/NitroEmulatorOutputStream.h"
#include "logger/NullLogger.h"
#include "MemoryEmulator/Arm/ArmDispatchTable.h"
#include "VirtualMachine/VMUndefinedArmTable.h"

ILogger* gLogger;

static void initLogger()
{
    std::unique_ptr<IOutputStream> outputStream;
    if (Environment::IsIsNitroEmulator() && Environment::SupportsAgbSemihosting())
    {
        outputStream = std::make_unique<NitroEmulatorOutputStream>();
    }
    else //if (Environment::SupportsNocashPrint())
    { 
        outputStream = std::make_unique<NocashOutputStream>();
    }
    // else
    // {
    //     gLogger = new NullLogger();
    //     return;
    // }
    gLogger = new PlainLogger(LogLevel::All, std::move(outputStream));
}

extern "C" void testMain(int argc, char* argv[])
{
    Environment::Initialize();

    initLogger();
    LOG_DEBUG("ARM9 Start\n");
    memu_initializeArmDispatchTable();
    vm_initializeUndefinedArmTable();
    testing::InitGoogleMock(&argc, argv);
    RUN_ALL_TESTS();

    // send semihosting exit command
    u32 agbMem = *(u32*)0x027FFF7C;
    *(vu16*)(agbMem + 0x10002) = 0xFF; // exit command
    *(vu16*)(agbMem + 0x10000) = 0x55;

    while (true);
}
