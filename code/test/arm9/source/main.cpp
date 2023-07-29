#include "common.h"
#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "core/Environment.h"
#include "logger/PlainLogger.h"
#include "logger/NocashOutputStream.h"
#include "logger/NitroEmulatorOutputStream.h"
#include "logger/NullLogger.h"

ILogger* gLogger;

static void initLogger()
{
    std::unique_ptr<IOutputStream> outputStream;
    if (Environment::IsIsNitroEmulator() && Environment::SupportsAgbSemihosting())
    {
        outputStream = std::make_unique<NitroEmulatorOutputStream>();
    }
    else if (Environment::SupportsNocashPrint())
    { 
        outputStream = std::make_unique<NocashOutputStream>();
    }
    else
    {
        gLogger = new NullLogger();
        return;
    }
    gLogger = new PlainLogger(LogLevel::All, std::move(outputStream));
}

extern "C" void testMain()
{
    Environment::Initialize();

    initLogger();
    LOG_DEBUG("ARM9 Start\n");
    testing::InitGoogleMock();
    RUN_ALL_TESTS();

    while (true);
}
