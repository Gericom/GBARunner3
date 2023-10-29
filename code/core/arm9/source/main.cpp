#include "common.h"
#include <libtwl/mem/memVram.h>
#include <libtwl/mem/memNtrWram.h>
#include <libtwl/mem/memExtern.h>
#include <libtwl/sys/sysPower.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxBackground.h>
#include <libtwl/gfx/gfxWindow.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <array>
#include <algorithm>
#include <string.h>
#include "cp15.h"
#include "Fat/ff.h"
#include "DsDefinitions.h"
#include "VirtualMachine/VirtualMachine.h"
#include "Emulator/IoRegisters.h"
#include "Core/Environment.h"
#include "Peripherals/DmaTransfer.h"
#include "Logger/NitroEmulatorOutputStream.h"
#include "Logger/PlainLogger.h"
#include "Logger/NullLogger.h"
#include "Save/SaveTagScanner.h"
#include "Save/Save.h"
#include "SdCache/SdCache.h"
#include "JitPatcher/JitCommon.h"
#include "JitPatcher/JitArm.h"
#include "Peripherals/Sound/GbaSound9.h"
#include "Patches/HarvestMoonPatches.h"
#include "Patches/BadMixerPatch.h"
#include "Application/Settings/AppSettingsService.h"
#include "SystemIpcCommand.h"
#include "IpcChannels.h"
#include "GbaHeader.h"
#include "MemoryEmulator/MemoryLoadStore.h"
#include "ColorLut.h"
#include "MemoryProtectionUnit.h"

#define DEFAULT_ROM_FILE_PATH           "/rom.gba"
#define BIOS_FILE_PATH                  "/_gba/bios.bin"
#define SETTINGS_FILE_PATH              "/_gba/gbarunner3.json"
#define GAME_SETTINGS_FILE_PATH_FORMAT  "/_gba/configs/%c%c%c%c%02X.json"

[[gnu::section(".ewram.bss")]]
FATFS gFatFs;
[[gnu::section(".ewram.bss")]]
FIL gFile;

u32 gGbaBios[16 * 1024 / 4] alignas(256);

static NitroEmulatorOutputStream sIsNitroOutput;
static PlainLogger sPlainLogger { LogLevel::All, &sIsNitroOutput };
static NullLogger sNullLogger;
ILogger* gLogger;

static void setupLogger()
{
    if (Environment::IsIsNitroEmulator())
        gLogger = &sPlainLogger;
    else
        gLogger = &sNullLogger;
}

static bool mountDldi()
{
    FRESULT res = f_mount(&gFatFs, "fat:", 1);
    if (res != FR_OK)
    {
        gLogger->Log(LogLevel::Error, "Mounting dldi failed\n");
        return false;
    }
    f_chdrive("fat:");
    return true;
}

static bool mountDsiSd()
{
    FRESULT res = f_mount(&gFatFs, "sd:", 1);
    if (res != FR_OK)
    {
        gLogger->Log(LogLevel::Error, "Mounting DSi sd failed\n");
        return false;
    }
    f_chdrive("sd:");
    return true;
}

static bool mountAgbSemihosting()
{
    FRESULT res = f_mount(&gFatFs, "pc:", 1);
    if (res != FR_OK)
    {
        gLogger->Log(LogLevel::Error, "Mounting agb semihosting failed\n");
        return false;
    }
    f_chdrive("pc:");
    return true;
}

static void loadGbaBios()
{
    memset(&gFile, 0, sizeof(gFile));
    f_open(&gFile, BIOS_FILE_PATH, FA_OPEN_EXISTING | FA_READ);
    UINT br;
    f_read(&gFile, gGbaBios, 16 * 1024, &br);
    f_close(&gFile);
}

static constexpr auto sBiosRelocations = std::to_array<u16>
({
    0x027C, 0x0AB8, 0x0ABC, 0x0AC4, 0x0ACC, 0x0ADC,
    0x0AE0, 0x0AE4, 0x0AEC, 0x0AF0, 0x0B0C, 0x0B2C,
    0x1430, 0x16F8, 0x16FC, 0x1700, 0x1788, 0x1924,
    0x1D64, 0x1D6C, 0x1D80, 0x1D84, 0x1D90, 0x1D9C,
    0x23A4, 0x2624, 0x26C0, 0x2C14, 0x30AC, 0x37D4,
    0x37D8, 0x37E0, 0x37E4, 0x381C, 0x3820, 0x3824,
    0x3828, 0x38A0, 0x38A4, 0x38A8, 0x38AC, 0x38B0,
    0x390C, 0x3910, 0x3914, 0x3918, 0x391C, 0x3920,
    0x3924, 0x3984, 0x3988, 0x398C, 0x3990, 0x3994,
    0x3998, 0x399C, 0x39C4, 0x39C8, 0x39CC
});

static void relocateGbaBios()
{
    const u32 base = (u32)gGbaBios;
    //swi table
    for (int i = 0; i < 43; i++)
        gGbaBios[(0x01C8 >> 2) + i] += base;
    *(vu16*)(((u8*)gGbaBios) + 0x868) = 0; // prevent address check fail
    for (int i = 0; i < 38; i++)
        gGbaBios[(0x3738 >> 2) + i] += base;
    for (u16 address : sBiosRelocations)
        gGbaBios[address >> 2] += base;
}

static void applyBiosVmPatches()
{
    gGbaBios[0x0024 >> 2] = 0xE1E0009C; // mrs r12, spsr
    gGbaBios[0x0028 >> 2] = 0xE1A0009E; // mrs lr, cpsr
    gGbaBios[0x005C >> 2] = 0xE1C9009C; // msr spsr_cf, r12
    gGbaBios[0x0064 >> 2] = 0xEE64008E; // subs pc, lr, #4
    gGbaBios[0x007C >> 2] = 0x01A0009C; // mrseq r12, cpsr
    gGbaBios[0x0084 >> 2] = 0x0189009C; // msreq cpsr_cf, r12
    gGbaBios[0x0090 >> 2] = 0xE1890090; // msr cpsr_cf, r0
    gGbaBios[0x00D4 >> 2] = 0xE1890090; // msr cpsr_cf, r0
    gGbaBios[0x00E4 >> 2] = 0xE1890090; // msr cpsr_cf, r0
    gGbaBios[0x00F0 >> 2] = 0xE1C9009E; // msr spsr_cf, lr
    gGbaBios[0x00F8 >> 2] = 0xE1890090; // msr cpsr_cf, r0
    gGbaBios[0x0104 >> 2] = 0xE1C9009E; // msr spsr_cf, lr
    gGbaBios[0x010C >> 2] = 0xE1890090; // msr cpsr_cf, r0
    gGbaBios[0x013C >> 2] = 0xEE64008E; // subs pc, lr, #4
    gGbaBios[0x0150 >> 2] = 0xE1E0009B; // mrs r11, spsr
    gGbaBios[0x0160 >> 2] = 0xE189009B; // msr cpsr_cf, r11
    gGbaBios[0x0178 >> 2] = 0xE189009C; // msr cpsr_cf, r12
    gGbaBios[0x0180 >> 2] = 0xE1C9009B; // msr spsr_cf, r11
    gGbaBios[0x0188 >> 2] = 0xEE64000E; // movs pc, lr
    gGbaBios[0x0388 >> 2] = 0xE189009C; // msr cpsr_cf, r12
}

static void applyBiosJitPatches()
{
    gGbaBios[0x00DC >> 2] = 0xE1B0009E; // bx lr (jump to rom)
    gGbaBios[0x0134 >> 2] = 0xEE800090; // ldr pc, [r0, #-4] (jump to irq handler)
}

static void loadGbaRom(const char* romPath)
{
    UINT br;
    memset(&gFile, 0, sizeof(gFile));
    f_open(&gFile, romPath, FA_OPEN_EXISTING | FA_READ);
    sdc_init();
    f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);

    auto header = reinterpret_cast<const GbaHeader*>(0x02200000);
    HarvestMoonPatches().TryApplyPatches(header->gameCode);
    if (BadMixerPatch().TryApplyPatch())
    {
        gLogger->Log(LogLevel::Debug, "Bad mixer patch applied\n");
    }

    // f_open(&gFile, "/suite.gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, f_size(&gFile), &br);
    // // agb aging
    // // *(vu32*)0x022000C4 = 0xE1890090; // msr cpsr_cf, r0
    // // mgba suite
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02250010 = 0xE1E00090; // mrs r0, spsr
    // *(vu32*)0x02250068 = 0xE1A00091; // mrs r1, cpsr
    // *(vu32*)0x02250074 = 0xE1890091; // msr cpsr_cf, r1
    // *(vu32*)0x02250094 = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x022500A0 = 0xE1890093; // msr cpsr_cf, r3
    // *(vu32*)0x022500AC = 0xE1C90090; // msr spsr_cf, r0
    // *(vu32*)0x022500B0 = 0xEE64000E; // movs pc, lr

    // f_open(&gFile, "/Mario Kart - Super Circuit (Europe).gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // mksc eu
    // *(vu32*)0x022000C0 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000D0 = 0xE1890090; // msr cpsr_cf, r0

    // f_open(&gFile, "/Sims, The - Bustin' Out (USA, Europe) (En,Fr,De,Es,It,Nl).gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // *(vu32*)0x022000C4 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000D0 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02200134 = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x0220013C = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x0220014C = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x02200154 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02200170 = 0xE1E00090; // mrs r0, spsr
    // *(vu32*)0x02200238 = 0xE1A00093; // mrs r3, cpsr
    // *(vu32*)0x02200244 = 0xE1890093; // msr cpsr_cf, r3
    // *(vu32*)0x02200264 = 0xE1A00093; // mrs r3, cpsr
    // *(vu32*)0x02200270 = 0xE1890093; // msr cpsr_cf, r3
    // *(vu32*)0x02200280 = 0xE1C90090; // msr spsr_cf, r0

    // f_open(&gFile, "/Asterix & Obelix XXL (Europe) (En,Fr,De,Es,It,Nl).gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // *(vu32*)0x02250000 = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x0225000C = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02250014 = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x02250020 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02250028 = 0xE1A00090; // mrs r0, cpsr
    // *(vu32*)0x02250034 = 0xE1890090; // msr cpsr_cf, r0

    // f_open(&gFile, "/DK - King of Swing (Europe) (En,Fr,De,Es,It).gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // *(vu32*)0x022000C4 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000D0 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02200414 = 0xE1E00090; // mrs r0, spsr
    // *(vu32*)0x02200460 = 0xE1A00093; // mrs r3, cpsr
    // *(vu32*)0x0220046C = 0xE1890093; // msr cpsr_cf, r3
    // *(vu32*)0x02200488 = 0xE1A00093; // mrs r3, cpsr
    // *(vu32*)0x02200494 = 0xE1890093; // msr cpsr_cf, r3
    // *(vu32*)0x022004A4 = 0xE1C90090; // msr spsr_cf, r0

    // f_open(&gFile, "/Super Mario Advance 4 - Super Mario Bros. 3 (USA).gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // *(vu32*)0x022000C4 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000D0 = 0xE1890090; // msr cpsr_cf, r0

    // f_open(&gFile, "/Banjo Kazooie - Grunty's Revenge # GBA.GBA", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, 2 * 1024 * 1024, &br);
    // *(vu32*)0x02200160 = 0xE1810090; // msr cpsr_c, r0
    // *(vu32*)0x02200180 = 0xE1810090; // msr cpsr_c, r0
    // *(vu32*)0x02200AAC = 0x91890090; // msrls cpsr_fc, r0
    // *(vu32*)0x02200AC8 = 0x91890090; // msrls cpsr_fc, r0
    // *(vu32*)0x02200B04 = 0x91890091; // msrls cpsr_fc, r1

    // f_open(&gFile, "/gba-niccc.gba", FA_OPEN_EXISTING | FA_READ);
    // f_read(&gFile, (void*)0x02200000, f_size(&gFile), &br);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x027A426C = 0xE1E00092; // mrs r2, spsr
    // *(vu32*)0x027A4274 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x027A4280 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x027A429C = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x027A42A8 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x027A42B0 = 0xE1C90092; // msr spsr_cf, r2

    // f_open(&gFile, "/varooom-3d_bad_audio.gba", FA_OPEN_EXISTING | FA_READ);
    // sdc_init();
    // f_read(&gFile, (void*)0x02200000, f_size(&gFile), &br);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02467968 = 0xE1E00092; // mrs r2, spsr
    // *(vu32*)0x02467970 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x0246797C = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x02467998 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x024679A4 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x024679AC = 0xE1C90092; // msr spsr_cf, r2

    // f_open(&gFile, "/dma_demo.gba", FA_OPEN_EXISTING | FA_READ);
    // f_read(&gFile, (void*)0x02200000, f_size(&gFile), &br);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
}

static void handleSave(const char* savePath)
{
    u32 tagRomAddress;
    const SaveTypeInfo* saveTypeInfo = SaveTagScanner().FindSaveTag(&gFile, &sdc_cache[0][0], tagRomAddress);
    
    if (saveTypeInfo)
    {
        gLogger->Log(LogLevel::Debug, "%s\n", saveTypeInfo->tag);
        if (saveTypeInfo->patchFunc)
        {
            if (!saveTypeInfo->patchFunc(saveTypeInfo, &gFile, tagRomAddress, &sdc_cache[0][0]))
            {
                gLogger->Log(LogLevel::Error, "Save patching failed\n");
            }
        }

        if ((saveTypeInfo->type & SAVE_TYPE_MASK) == SAVE_TYPE_EEPROM)
        {
            memu_itcmLoad8Table[0xE] = (void*)memu_load8Undefined;
            memu_itcmLoad8Table[0xF] = (void*)memu_load8Undefined;
            memu_load8Table[0xE] = (void*)memu_load8Undefined;
            memu_load8Table[0xF] = (void*)memu_load8Undefined;
            memu_itcmLoad16Table[0xE] = (void*)memu_load16Undefined;
            memu_itcmLoad16Table[0xF] = (void*)memu_load16Undefined;
            memu_load16Table[0xE] = (void*)memu_load16Undefined;
            memu_load16Table[0xF] = (void*)memu_load16Undefined;
            memu_itcmLoad32Table[0xE] = (void*)memu_load32Undefined;
            memu_itcmLoad32Table[0xF] = (void*)memu_load32Undefined;
            memu_load32Table[0xE] = (void*)memu_load32Undefined;
            memu_load32Table[0xF] = (void*)memu_load32Undefined;
        }
        if ((saveTypeInfo->type & SAVE_TYPE_MASK) == SAVE_TYPE_EEPROM ||
            (saveTypeInfo->type & SAVE_TYPE_MASK) == SAVE_TYPE_FLASH)
        {
            memu_itcmStore8Table[0xE] = (void*)memu_store8Undefined;
            memu_itcmStore8Table[0xF] = (void*)memu_store8Undefined;
            memu_store8Table[0xE] = (void*)memu_store8Undefined;
            memu_store8Table[0xF] = (void*)memu_store8Undefined;
            memu_itcmStore16Table[0xE] = (void*)memu_store16Undefined;
            memu_itcmStore16Table[0xF] = (void*)memu_store16Undefined;
            memu_store16Table[0xE] = (void*)memu_store16Undefined;
            memu_store16Table[0xF] = (void*)memu_store16Undefined;
            memu_itcmStore32Table[0xE] = (void*)memu_store32Undefined;
            memu_itcmStore32Table[0xF] = (void*)memu_store32Undefined;
            memu_store32Table[0xE] = (void*)memu_store32Undefined;
            memu_store32Table[0xF] = (void*)memu_store32Undefined;
        }
    }

    sav_initializeSave(saveTypeInfo, savePath);
}

// extern "C" void logOpcode(u32 opcode)
// {
//     gLogger->Log(LogLevel::Trace, "0x%X\n", opcode);
// }

extern "C" void logAddress(u32 address)
{
    gLogger->Log(LogLevel::Trace, "0x%X\n", address);
}

static bool shouldMountDsiSd(int argc, char* argv[])
{
    if (!Environment::IsDsiMode())
        return false;

    if (argc == 0)
        return true;

    if (argv[0][0] == 'f' && argv[0][1] == 'a' && argv[0][2] == 't' && argv[0][3] == ':')
        return false;

    return true;
}

static void setTopBacklight(bool enabled)
{
    ipc_sendWordDirect(
        ((enabled ? 1 : 0) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
        (SYSTEM_IPC_CMD_SET_TOP_BACKLIGHT << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_SYSTEM);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

static void setBottomBacklight(bool enabled)
{
    ipc_sendWordDirect(
        ((enabled ? 1 : 0) << (IPC_FIFO_MSG_CHANNEL_BITS + 4)) |
        (SYSTEM_IPC_CMD_SET_BOTTOM_BACKLIGHT << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_SYSTEM);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

static void setupCaptureOam()
{
    const auto& displaySettings = gAppSettingsService.GetAppSettings().displaySettings;
    vu16* oamPtr = GFX_OAM_SUB;
    for (u32 y = 0; y < NDS_LCD_HEIGHT; y += 64)
    {
        for (u32 x = 0; x < NDS_LCD_WIDTH; x += 64)
        {
            oamPtr[0] =
                GFX_OAM_ATTR0_SHAPE_64_64 |
                GFX_OAM_ATTR0_MODE_BITMAP |
                GFX_OAM_ATTR0_Y(y + displaySettings.centerOffsetY);
            oamPtr[1] =
                GFX_OAM_ATTR1_SIZE_64_64 |
                GFX_OAM_ATTR1_X(x + displaySettings.centerOffsetX);
            oamPtr[2] =
                GFX_OAM_ATTR2_BMP_ALPHA(15) |
                GFX_OAM_ATTR2_VRAM_OFFS(((y >> 3) << 5) + (x >> 3));
            oamPtr += 4;
        }
    }
}

static void setupCenterAndMask()
{
    const auto& displaySettings = gAppSettingsService.GetAppSettings().displaySettings;
    u32 windowX0 = std::clamp<u32>(displaySettings.centerOffsetX, 0, NDS_LCD_WIDTH - 1);
    u32 windowY0 = std::clamp<u32>(displaySettings.centerOffsetY, 0, NDS_LCD_HEIGHT - 1);
    u32 windowX1 = std::clamp<u32>(windowX0 + displaySettings.maskWidth, 1, NDS_LCD_WIDTH);
    u32 windowY1 = std::clamp<u32>(windowY0 + displaySettings.maskHeight, 1, NDS_LCD_HEIGHT);
    if (windowX0 == 0 && windowX1 == NDS_LCD_WIDTH)
    {
        // use two windows to cover a width of 256
        gfx_setSubWindow0(0, windowY0, 255, windowY1);
        gfx_setSubWindow1(255, windowY0, 0, windowY1);
    }
    else
    {
        gfx_setSubWindow0(windowX0, windowY0, windowX1, windowY1);
        gfx_setSubWindow1(windowX0, windowY0, windowX1, windowY1);
    }

    REG_WININ_SUB = ((1 << 3) | (1 << 4)) | (((1 << 3) | (1 << 4)) << 8);
    REG_WINOUT_SUB = 0;
    REG_DISPCNT_SUB = 0x40017923;
    REG_BG3CNT_SUB = 0x4084;
    gfx_setSubBg3Affine(
        256, 0, 0, 256,
        -(displaySettings.centerOffsetX * 256),
        -(displaySettings.centerOffsetY * 256));

    setupCaptureOam();
}

static void setupGbaScreen()
{
    const auto& displaySettings = gAppSettingsService.GetAppSettings().displaySettings;
    if (displaySettings.gbaScreen == GbaScreen::Top)
    {
        if (displaySettings.enableCenterAndMask)
            sys_setMainEngineToBottomScreen();
        else
            sys_setMainEngineToTopScreen();
        setBottomBacklight(false);
    }
    else
    {
        if (displaySettings.enableCenterAndMask)
            sys_setMainEngineToTopScreen();
        else
            sys_setMainEngineToBottomScreen();
        setTopBacklight(false);
    }

    REG_DISPCAPCNT = 0x00320000;
    if (displaySettings.enableCenterAndMask)
        setupCenterAndMask();
}

static void setupColorCorrection()
{
    if (gAppSettingsService.GetAppSettings().displaySettings.gbaColorCorrection == GbaColorCorrection::None)
    {
        clut_disableColorCorrection();
    }
}

static void setupGbaScreenBrightness()
{
    const auto& displaySettings = gAppSettingsService.GetAppSettings().displaySettings;
    if (displaySettings.enableCenterAndMask)
        REG_MASTER_BRIGHT_SUB = 0x8000 | (16 - displaySettings.gbaScreenBrightness);
    else
        REG_MASTER_BRIGHT = 0x8000 | (16 - displaySettings.gbaScreenBrightness);
}

static void applyGameJitPatches()
{
    gLogger->Log(LogLevel::Debug, "Applying game JIT patches...\n");

    const auto& runSettings = gAppSettingsService.GetAppSettings().runSettings;
    for (u32 i = 0; i < runSettings.jitPatchAddressCount; ++i)
    {
        u32 jitPatchAddress = runSettings.jitPatchAddresses[i];
        if (jitPatchAddress >= 0x08000000u && jitPatchAddress < 0x0A000000u)
        {
            gLogger->Log(LogLevel::Debug, "0x%08X\n", jitPatchAddress);
            if (jitPatchAddress < 0x08200000u)
            {
                jit_processArmInstruction(reinterpret_cast<u32*>(jitPatchAddress - 0x08000000u + 0x02200000u));
            }

            jit_processArmInstruction(static_cast<u32*>(sdc_loadRomBlockForPatching(jitPatchAddress)));
        }
    }
}

static void setupJit()
{
    jit_init();

    const auto& runSettings = gAppSettingsService.GetAppSettings().runSettings;
    if (runSettings.jitPatchAddresses && runSettings.jitPatchAddressCount > 0)
    {
        // manual jit patches
        applyGameJitPatches();
        jit_disable();
    }
    else
    {
        // jit enabled
        applyBiosJitPatches();
    }
}

static void setupWramInstructionCache()
{
    const auto& runSettings = gAppSettingsService.GetAppSettings().runSettings;
    mpu_setRegionICacheEnable(MPU_REGION_6, runSettings.enableWramInstructionCache);
    mpu_setRegionICacheEnable(MPU_REGION_7, runSettings.enableWramInstructionCache);
}

static void loadGameSpecificSettings()
{
    auto header = reinterpret_cast<const GbaHeader*>(0x02200000);
    auto path = std::make_unique<char[]>(128);
    mini_snprintf(path.get(), 128, GAME_SETTINGS_FILE_PATH_FORMAT,
        header->gameCode & 0xFF, (header->gameCode >> 8) & 0xFF,
        (header->gameCode >> 16) & 0xFF, header->gameCode >> 24,
        header->softwareVersion);

    if (gAppSettingsService.TryLoadAppSettings(path.get()))
    {
        gLogger->Log(LogLevel::Debug, "Loaded game specific settings from %s\n", path.get());
    }
}

extern "C" void gbaRunnerMain(int argc, char* argv[])
{
    heap_init();

    REG_DISPCNT = 0x10000;
    REG_DISPCNT_SUB = 0x10000;
    GFX_PLTT_BG_MAIN[0] = 0x1F;
    GFX_PLTT_BG_SUB[0] = 0;
    REG_MASTER_BRIGHT = 0;

    mem_setVramCMapping(MEM_VRAM_C_LCDC);
    mem_setVramDMapping(MEM_VRAM_D_LCDC);
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    mem_setVramFMapping(MEM_VRAM_FG_MAIN_OBJ_00000);
    mem_setVramGMapping(MEM_VRAM_FG_MAIN_OBJ_04000);
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM9, MEM_NTR_WRAM_ARM9);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);
    sys_set3DGeometryEnginePower(true); // enable geometry engine to generate gx fifo irq

    Environment::Initialize();
    setupLogger();

    mem_setMainMemoryPriority(EXMEMCNT_MAIN_MEM_PRIO_ARM9);

    bool mountResult;
    if (shouldMountDsiSd(argc, argv))
        mountResult = mountDsiSd();
    else
        mountResult = mountDldi();

    if (!mountResult)
    {
        GFX_PLTT_BG_MAIN[0] = 0x1F << 10;
        while (1);
    }

    // if (Environment::SupportsAgbSemihosting())
        // mountAgbSemihosting();

    gAppSettingsService.TryLoadAppSettings(SETTINGS_FILE_PATH);

    loadGbaBios();
    relocateGbaBios();
    applyBiosVmPatches();
    const char* romPath = argc > 1 ? argv[1] : DEFAULT_ROM_FILE_PATH;
    loadGbaRom(romPath);
    char* romExtension = strrchr(romPath, '.');
    if (romExtension)
    {
        romExtension[1] = 's';
        romExtension[2] = 'a';
        romExtension[3] = 'v';
        romExtension[4] = '\0';
    }
    handleSave(romPath);

    setupGbaScreen();
    setupColorCorrection();
    setupGbaScreenBrightness();
    loadGameSpecificSettings();

    GFX_PLTT_BG_MAIN[0] = 0x1F << 5;
    // while (((*(vu16*)0x04000130) & 1) == 1);
    // Do not clear ewram before we read argv
    memset((void*)0x02000000, 0, 256 * 1024);
    memset((void*)0x03000000, 0, 32 * 1024);
    memset((void*)GFX_BG_MAIN, 0, 64 * 1024);
    memset((void*)GFX_OBJ_MAIN, 0, 32 * 1024);
    memset(emu_ioRegisters, 0, sizeof(emu_ioRegisters));
    setupJit();
    dma_init();
    gbas_init();
    dc_flushRange((void*)0x02200000, 0x400000);
    dc_flushRange(gGbaBios, sizeof(gGbaBios));
    ic_invalidateAll();
    setupWramInstructionCache();

    rtos_setIrqMask(RTOS_IRQ_VBLANK);
    rtos_ackIrqMask(~0u);
    REG_IME = 1;
    gfx_setVBlankIrqEnabled(true);
    VirtualMachine virtualMachine { &gGbaBios[0], &gGbaBios[8 >> 2], &gGbaBios[0x18 >> 2] };
    context_t runContext { };
    virtualMachine.Run(&runContext);
    while (true);
}
