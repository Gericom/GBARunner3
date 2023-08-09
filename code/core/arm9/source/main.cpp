#include "common.h"
#include "Fat/ff.h"
#include "VirtualMachine/VirtualMachine.h"
#include "Core/Environment.h"

FATFS gFatFs;
static FIL sFile;

u32 gGbaBios[16 * 1024 / 4];

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
    f_open(&sFile, "/_gba/bios.bin", FA_OPEN_EXISTING | FA_READ);
    UINT br;
    f_read(&sFile, gGbaBios, 16 * 1024, &br);
    f_close(&sFile);
}

static void relocateGbaBios()
{
    const u32 base = (u32)gGbaBios;
	//swi table
	for (int i = 0; i < 43; i++)
		gGbaBios[(0x01C8 >> 2) + i] += base;
	gGbaBios[0x027C >> 2] += base;
	gGbaBios[0x0AB8 >> 2] += base;
	gGbaBios[0x0ABC >> 2] += base;
	gGbaBios[0x0AC4 >> 2] += base;
	gGbaBios[0x0ACC >> 2] += base;
	gGbaBios[0x0ADC >> 2] += base;
	gGbaBios[0x0AE0 >> 2] += base;
	gGbaBios[0x0AE4 >> 2] += base;
	gGbaBios[0x0AEC >> 2] += base;
	gGbaBios[0x0AF0 >> 2] += base;
	gGbaBios[0x0B0C >> 2] += base;
	gGbaBios[0x0B2C >> 2] += base;
	gGbaBios[0x1430 >> 2] += base;
	gGbaBios[0x16F8 >> 2] += base;
	gGbaBios[0x16FC >> 2] += base;
	gGbaBios[0x1700 >> 2] += base;
	gGbaBios[0x1788 >> 2] += base;
	gGbaBios[0x1924 >> 2] += base;
	gGbaBios[0x1D64 >> 2] += base;
	gGbaBios[0x1D6C >> 2] += base;
	gGbaBios[0x1D80 >> 2] += base;
	gGbaBios[0x1D84 >> 2] += base;
	gGbaBios[0x1D90 >> 2] += base;
	gGbaBios[0x1D9C >> 2] += base;
	gGbaBios[0x23A4 >> 2] += base;
	gGbaBios[0x2624 >> 2] += base;
	gGbaBios[0x26C0 >> 2] += base;
	gGbaBios[0x2C14 >> 2] += base;
	gGbaBios[0x30AC >> 2] += base;
	*(vu16*)(((u8*)gGbaBios) + 0x868) = 0; // prevent address check fail
	for (int i = 0; i < 38; i++)
		gGbaBios[(0x3738 >> 2) + i] += base;
	gGbaBios[0x37D4 >> 2] += base;
	gGbaBios[0x37D8 >> 2] += base;
	gGbaBios[0x37E0 >> 2] += base;
	gGbaBios[0x37E4 >> 2] += base;
	gGbaBios[0x381C >> 2] += base;
	gGbaBios[0x3820 >> 2] += base;
	gGbaBios[0x3824 >> 2] += base;
	gGbaBios[0x3828 >> 2] += base;
	gGbaBios[0x38A0 >> 2] += base;
	gGbaBios[0x38A4 >> 2] += base;
	gGbaBios[0x38A8 >> 2] += base;
	gGbaBios[0x38AC >> 2] += base;
	gGbaBios[0x38B0 >> 2] += base;
	gGbaBios[0x390C >> 2] += base;
	gGbaBios[0x3910 >> 2] += base;
	gGbaBios[0x3914 >> 2] += base;
	gGbaBios[0x3918 >> 2] += base;
	gGbaBios[0x391C >> 2] += base;
	gGbaBios[0x3920 >> 2] += base;
	gGbaBios[0x3924 >> 2] += base;
	gGbaBios[0x3984 >> 2] += base;
	gGbaBios[0x3988 >> 2] += base;
	gGbaBios[0x398C >> 2] += base;
	gGbaBios[0x3990 >> 2] += base;
	gGbaBios[0x3994 >> 2] += base;
	gGbaBios[0x3998 >> 2] += base;
	gGbaBios[0x399C >> 2] += base;
	gGbaBios[0x39C4 >> 2] += base;
	gGbaBios[0x39C8 >> 2] += base;
	gGbaBios[0x39CC >> 2] += base;
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

extern "C" void gbaRunnerMain(void)
{
    *(vu32*)0x04000000 = 0x10000;
    *(vu32*)0x05000000 = 0x1F;
    *(vu32*)0x0400006C = 0;

    *(vu8*)0x04000247 = 0; // iwram to arm9
    Environment::Initialize();
    if (Environment::SupportsAgbSemihosting())
        mountAgbSemihosting();
    loadGbaBios();
    relocateGbaBios();
    applyBiosVmPatches();
    VirtualMachine virtualMachine { &gGbaBios[0], &gGbaBios[8 >> 2], &gGbaBios[0x18 >> 2] };
    context_t runContext { };
    virtualMachine.Run(&runContext);
    while (true);
}
