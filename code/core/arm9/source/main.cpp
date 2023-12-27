#include "common.h"
#include <libtwl/mem/memVram.h>
#include <libtwl/mem/memNtrWram.h>
#include <libtwl/mem/memExtern.h>
#include <libtwl/sys/sysPower.h>
#include <libtwl/gfx/gfx3d.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxBackground.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/rtos/rtosIrq.h>
#include <array>
#include <string.h>
#include "cp15.h"
#include "Fat/ff.h"
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
#include "GbaHeader.h"
#include "MemoryEmulator/MemoryLoadStore.h"
#include "ColorLut.h"
#include "MemoryProtectionConfiguration.h"
#include "MemoryProtectionUnit.h"
#include "MemoryEmulator/RomDefs.h"
#include "Application/GbaDisplayConfigurationService.h"
#include "Application/GbaBorderService.h"
#include "Patches/PatchSwi.h"
#include "Patches/SelfModifyingPatches.h"

#define DEFAULT_ROM_FILE_PATH           "/rom.gba"
#define BIOS_FILE_PATH                  "/_gba/bios.bin"
#define SETTINGS_FILE_PATH              "/_gba/gbarunner3.json"
#define GAME_SETTINGS_FILE_PATH_FORMAT  "/_gba/configs/%c%c%c%c%02X.json"

[[gnu::section(".ewram.bss")]]
FATFS gFatFs;
[[gnu::section(".ewram.bss")]]
FIL gFile;
[[gnu::section(".ewram.bss")]]
GbaHeader gRomHeader;

[[gnu::section(".vramhi.bss")]]
u32 gGbaBios[16 * 1024 / 4] alignas(256);

/// @brief Opcodes returned by bios reads.
u32 memu_biosOpcodes[4]
{
    0xE25EF004, // MEMU_BIOS_OPCODE_ID_IRQ_ENTRY
    0xE129F000, // MEMU_BIOS_OPCODE_ID_RESET
    0xE55EC002, // MEMU_BIOS_OPCODE_ID_IRQ_EXIT
    0xE3A02004  // MEMU_BIOS_OPCODE_ID_SWI_EXIT
};

static NitroEmulatorOutputStream sIsNitroOutput;
[[gnu::section(".ewram.bss")]]
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
    f_read(&gFile, &gRomHeader, sizeof(GbaHeader), &br);
    f_lseek(&gFile, ROM_LINEAR_GBA_ADDRESS - 0x08000000);
    f_read(&gFile, (void*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, &br);

    HarvestMoonPatches().TryApplyPatches(gRomHeader.gameCode);
    if (BadMixerPatch().TryApplyPatch())
    {
        gLogger->Log(LogLevel::Debug, "Bad mixer patch applied\n");
    }
}

static void disableSramReads(void)
{
    memu_itcmLoad8Table[0xE] = (void*)memu_load8Undefined;
    memu_itcmLoad8Table[0xF] = (void*)memu_load8Undefined;
    memu_load8Table[0xE] = (u32)memu_load8Undefined;
    memu_load8Table[0xF] = (u32)memu_load8Undefined;
    memu_itcmLoad16Table[0xE] = (void*)memu_load16Undefined;
    memu_itcmLoad16Table[0xF] = (void*)memu_load16Undefined;
    memu_load16Table[0xE] = (u32)memu_load16Undefined;
    memu_load16Table[0xF] = (u32)memu_load16Undefined;
    memu_itcmLoad32Table[0xE] = (void*)memu_load32Undefined;
    memu_itcmLoad32Table[0xF] = (void*)memu_load32Undefined;
    memu_load32Table[0xE] = (u32)memu_load32Undefined;
    memu_load32Table[0xF] = (u32)memu_load32Undefined;
}

static void disableSramWrites(void)
{
    memu_itcmStore8Table[0xE] = (void*)memu_store8Undefined;
    memu_itcmStore8Table[0xF] = (void*)memu_store8Undefined;
    memu_store8Table[0xE] = (u32)memu_store8Undefined;
    memu_store8Table[0xF] = (u32)memu_store8Undefined;
    memu_itcmStore16Table[0xE] = (void*)memu_store16Undefined;
    memu_itcmStore16Table[0xF] = (void*)memu_store16Undefined;
    memu_store16Table[0xE] = (u32)memu_store16Undefined;
    memu_store16Table[0xF] = (u32)memu_store16Undefined;
    memu_itcmStore32Table[0xE] = (void*)memu_store32Undefined;
    memu_itcmStore32Table[0xF] = (void*)memu_store32Undefined;
    memu_store32Table[0xE] = (u32)memu_store32Undefined;
    memu_store32Table[0xF] = (u32)memu_store32Undefined;
}

static void handleSave(const char* savePath)
{
    const auto& gameSettings = gAppSettingsService.GetAppSettings().gameSettings;
    if (gameSettings.saveType == GbaSaveType::None)
    {
        gLogger->Log(LogLevel::Debug, "Save Type: None\n");
        disableSramReads();
        disableSramWrites();
        return;
    }

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
            disableSramReads();
        }
        if ((saveTypeInfo->type & SAVE_TYPE_MASK) == SAVE_TYPE_EEPROM ||
            (saveTypeInfo->type & SAVE_TYPE_MASK) == SAVE_TYPE_FLASH)
        {
            disableSramWrites();
        }
    }

    sav_initializeSave(saveTypeInfo, savePath);
}

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
            if (jitPatchAddress >= ROM_LINEAR_GBA_ADDRESS && jitPatchAddress < ROM_LINEAR_END_GBA_ADDRESS)
            {
                jit_processArmInstruction(reinterpret_cast<u32*>(jitPatchAddress - ROM_LINEAR_GBA_ADDRESS + ROM_LINEAR_DS_ADDRESS));
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
    mpu_setRegionInstructionCacheEnable(MPU_REGION_GBA_IWRAM, runSettings.enableWramInstructionCache);
    mpu_setRegionInstructionCacheEnable(MPU_REGION_GBA_EWRAM, runSettings.enableWramInstructionCache);
}

static void setupEWramDataCache()
{
    const auto& runSettings = gAppSettingsService.GetAppSettings().runSettings;
    mpu_setRegionDataCacheEnable(MPU_REGION_GBA_EWRAM, runSettings.enableEWramDataCache);
    mpu_setRegionDataBufferability(MPU_REGION_GBA_EWRAM, false);
}

static void loadGameSpecificSettings()
{
    auto path = std::make_unique<char[]>(128);
    mini_snprintf(path.get(), 128, GAME_SETTINGS_FILE_PATH_FORMAT,
        gRomHeader.gameCode & 0xFF, (gRomHeader.gameCode >> 8) & 0xFF,
        (gRomHeader.gameCode >> 16) & 0xFF, gRomHeader.gameCode >> 24,
        gRomHeader.softwareVersion);

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

    mem_setVramBMapping(MEM_VRAM_AB_MAIN_BG_40000);
    mem_setVramCMapping(MEM_VRAM_C_LCDC);
    mem_setVramDMapping(MEM_VRAM_D_LCDC);
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    mem_setVramFMapping(MEM_VRAM_FG_MAIN_OBJ_00000);
    mem_setVramGMapping(MEM_VRAM_FG_MAIN_OBJ_04000);
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM9, MEM_NTR_WRAM_ARM9);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);
    sys_set3DGeometryEnginePower(true); // enable geometry engine to generate gx fifo irq
    REG_GXSTAT = 0; // gx fifo irqs must be off

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

    patch_resetSwiPatches();
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
    loadGameSpecificSettings();
    handleSave(romPath);
    SelfModifyingPatches().ApplyPatches(gAppSettingsService.GetAppSettings().runSettings);

    const auto& displaySettings = gAppSettingsService.GetAppSettings().displaySettings;
    gGbaDisplayConfigurationService.ApplyDisplaySettings(displaySettings);
    if (displaySettings.enableCenterAndMask)
    {
        gGbaBorderService.SetupBorder(displaySettings.borderImage, gRomHeader.gameCode);
    }

    GFX_PLTT_BG_MAIN[0] = 0x1F << 5;
    // Do not clear ewram before we read argv
    memset((void*)0x02000000, 0, 256 * 1024);
    memset((void*)0x03000000, 0, 32 * 1024);
    memset((void*)GFX_BG_MAIN, 0, 64 * 1024);
    memset((void*)((u32)GFX_BG_MAIN + 0x40000), 0, 128 * 1024); // vram B
    memset((void*)GFX_OBJ_MAIN, 0, 32 * 1024);
    memset(emu_ioRegisters, 0, sizeof(emu_ioRegisters));
    setupJit();
    dma_init();
    gbas_init();
    dc_flushRange((void*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE);
    dc_flushRange(gGbaBios, sizeof(gGbaBios));
    ic_invalidateAll();
    setupWramInstructionCache();
    setupEWramDataCache();

    rtos_setIrqMask(RTOS_IRQ_VBLANK);
    rtos_ackIrqMask(~0u);
    REG_IME = 1;
    gfx_setVBlankIrqEnabled(true);
    VirtualMachine virtualMachine { &gGbaBios[0], &gGbaBios[8 >> 2], &gGbaBios[0x18 >> 2] };
    context_t runContext { };
    virtualMachine.Run(&runContext);
    while (true);
}
