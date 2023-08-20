#include "common.h"
#include <libtwl/mem/memVram.h>
#include <libtwl/mem/memNtrWram.h>
#include <libtwl/sys/sysPower.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfx2d.h>
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

[[gnu::section(".ewram.bss")]]
FATFS gFatFs;
[[gnu::section(".ewram.bss")]]
static FIL sFile;

u32 gGbaBios[16 * 1024 / 4] alignas(256);

static NitroEmulatorOutputStream sIsNitroOutput;
static PlainLogger sPlainLogger { LogLevel::All, &sIsNitroOutput };
ILogger* gLogger = &sPlainLogger;

static bool mountAgbSemihosting()
{
    FRESULT res = f_mount(&gFatFs, "pc:", 1);
    if (res != FR_OK)
        return false;
    f_chdrive("pc:");
    return true;
}

static void loadGbaBios()
{
    memset(&sFile, 0, sizeof(sFile));
    f_open(&sFile, "/_gba/bios.bin", FA_OPEN_EXISTING | FA_READ);
    UINT br;
    f_read(&sFile, gGbaBios, 16 * 1024, &br);
    f_close(&sFile);
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

static void loadAgbAging()
{
    memset(&sFile, 0, sizeof(sFile));
    // f_open(&sFile, "/suite.gba", FA_OPEN_EXISTING | FA_READ);
    // UINT br;
    // f_read(&sFile, (void*)0x02200000, f_size(&sFile), &br);
    // f_close(&sFile);
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

    f_open(&sFile, "/Mario Kart - Super Circuit (Europe).gba", FA_OPEN_EXISTING | FA_READ);
    UINT br;
    f_read(&sFile, (void*)0x02200000, 4 * 1024 * 1024, &br);
    f_close(&sFile);
    // mksc eu
    *(vu32*)0x022000C0 = 0xE1890090; // msr cpsr_cf, r0
    *(vu32*)0x022000D0 = 0xE1890090; // msr cpsr_cf, r0

    // f_open(&sFile, "/gba-niccc.gba", FA_OPEN_EXISTING | FA_READ);
    // UINT br;
    // f_read(&sFile, (void*)0x02200000, f_size(&sFile), &br);
    // f_close(&sFile);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x027A426C = 0xE1E00092; // mrs r2, spsr
    // *(vu32*)0x027A4274 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x027A4280 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x027A429C = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x027A42A8 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x027A42B0 = 0xE1C90092; // msr spsr_cf, r2

    // f_open(&sFile, "/varooom-3d_bad_audio.gba", FA_OPEN_EXISTING | FA_READ);
    // UINT br;
    // f_read(&sFile, (void*)0x02200000, f_size(&sFile), &br);
    // f_close(&sFile);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x02467968 = 0xE1E00092; // mrs r2, spsr
    // *(vu32*)0x02467970 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x0246797C = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x02467998 = 0xE1A00092; // mrs r2, cpsr
    // *(vu32*)0x024679A4 = 0xE1890092; // msr cpsr_cf, r2
    // *(vu32*)0x024679AC = 0xE1C90092; // msr spsr_cf, r2

    // f_open(&sFile, "/dma_demo.gba", FA_OPEN_EXISTING | FA_READ);
    // UINT br;
    // f_read(&sFile, (void*)0x02200000, f_size(&sFile), &br);
    // f_close(&sFile);
    // *(vu32*)0x022000EC = 0xE1890090; // msr cpsr_cf, r0
    // *(vu32*)0x022000F8 = 0xE1890090; // msr cpsr_cf, r0
}

// extern "C" void logOpcode(u32 opcode)
// {
//     gLogger->Log(LogLevel::Trace, "0x%X\n", opcode);
// }

extern "C" void gbaRunnerMain(void)
{
    *(vu32*)0x04000000 = 0x10000;
    GFX_PLTT_BG_MAIN[0] = 0x1F;
    *(vu32*)0x0400006C = 0;

    mem_setVramCMapping(MEM_VRAM_C_LCDC);
    mem_setVramDMapping(MEM_VRAM_D_LCDC);
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    mem_setVramFMapping(MEM_VRAM_FG_MAIN_OBJ_00000);
    mem_setVramGMapping(MEM_VRAM_FG_MAIN_OBJ_04000);
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM9, MEM_NTR_WRAM_ARM9);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);
    sys_set3DGeometryEnginePower(true); // enable geometry engine to generate gx fifo irq
    memset((void*)0x02000000, 0, 256 * 1024);
    memset((void*)0x03000000, 0, 32 * 1024);
    memset((void*)GFX_BG_MAIN, 0, 64 * 1024);
    memset((void*)GFX_OBJ_MAIN, 0, 32 * 1024);
    Environment::Initialize();
    if (Environment::SupportsAgbSemihosting())
        mountAgbSemihosting();
    loadGbaBios();
    relocateGbaBios();
    applyBiosVmPatches();
    loadAgbAging();
    GFX_PLTT_BG_MAIN[0] = 0x1F << 5;
    while (((*(vu16*)0x04000130) & 1) == 1);
    memset(emu_ioRegisters, 0, sizeof(emu_ioRegisters));
    dma_init();
    dc_flushRange((void*)0x02200000, 0x400000);
    dc_flushRange(gGbaBios, sizeof(gGbaBios));
    ic_invalidateAll();

    rtos_setIrqMask(RTOS_IRQ_VBLANK);
    rtos_ackIrqMask(~0u);
    REG_IME = 1;
    gfx_setVBlankIrqEnabled(true);
    VirtualMachine virtualMachine { &gGbaBios[0], &gGbaBios[8 >> 2], &gGbaBios[0x18 >> 2] };
    context_t runContext { };
    virtualMachine.Run(&runContext);
    while (true);
}
