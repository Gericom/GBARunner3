#include "common.h"
#include <libtwl/mem/memVram.h>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxBackground.h>
#include "Emulator/IoRegisters.h"
#include "MemoryEmulator/MemoryLoadStore.h"
#include "GbaIoRegOffsets.h"

static void setVramLoad012(void)
{
    memu_itcmLoad8Table[6] = (void*)memu_load8Vram012;
    memu_load8Table[6] = (u32)memu_load8Vram012;
    memu_itcmLoad16Table[6] = (void*)memu_load16Vram012;
    memu_load16Table[6] = (u32)memu_load16Vram012;
    memu_itcmLoad32Table[6] = (void*)memu_load32Vram012;
    memu_load32Table[6] = (u32)memu_load32Vram012;
}

static void setVramLoad345(void)
{
    memu_itcmLoad8Table[6] = (void*)memu_load8Vram345;
    memu_load8Table[6] = (u32)memu_load8Vram345;
    memu_itcmLoad16Table[6] = (void*)memu_load16Vram345;
    memu_load16Table[6] = (u32)memu_load16Vram345;
    memu_itcmLoad32Table[6] = (void*)memu_load32Vram345;
    memu_load32Table[6] = (u32)memu_load32Vram345;
}

[[gnu::noinline]]
static void displayModeChange(u32 oldMode, u32 newMode)
{
    switch (newMode)
    {
        case 0:
        case 1:
        case 2:
        {
            mem_setVramFMapping(MEM_VRAM_FG_MAIN_OBJ_00000);
            setVramLoad012();
            memu_itcmStore8Table[6] = (void*)memu_store8Vram012;
            memu_store8Table[6] = (u32)memu_store8Vram012;
            memu_itcmStore16Table[6] = (void*)memu_store16Vram012;
            memu_store16Table[6] = (u32)memu_store16Vram012;
            memu_itcmStore32Table[6] = (void*)memu_store32Vram012;
            memu_store32Table[6] = (u32)memu_store32Vram012;
            REG_BG2CNT = *(u16*)&emu_ioRegisters[GBA_REG_OFFS_BG2CNT] & ~0x30;
            break;
        }
        case 3:
        {
            mem_setVramFMapping(MEM_VRAM_FG_MAIN_BG_10000);
            setVramLoad345();
            memu_itcmStore8Table[6] = (void*)memu_store8Vram3;
            memu_store8Table[6] = (u32)memu_store8Vram3;
            memu_itcmStore16Table[6] = (void*)memu_store16Vram3;
            memu_store16Table[6] = (u32)memu_store16Vram3;
            memu_itcmStore32Table[6] = (void*)memu_store32Vram3;
            memu_store32Table[6] = (u32)memu_store32Vram3;
            for (u32 i = 0; i < 0x14000; i += 4)
            {
                memu_store32FromC(0x06000000 + i, *(vu32*)(0x06000000 + i));
            }
            u32 priority = *(u16*)&emu_ioRegisters[GBA_REG_OFFS_BG2CNT] & 3;
            REG_BG2CNT = 0x5084 | priority;
            break;
        }
        case 4:
        {
            mem_setVramFMapping(MEM_VRAM_FG_MAIN_BG_10000);
            setVramLoad345();
            memu_itcmStore8Table[6] = (void*)memu_store8Vram4;
            memu_store8Table[6] = (u32)memu_store8Vram4;
            memu_itcmStore16Table[6] = (void*)memu_store16Vram4;
            memu_store16Table[6] = (u32)memu_store16Vram4;
            memu_itcmStore32Table[6] = (void*)memu_store32Vram4;
            memu_store32Table[6] = (u32)memu_store32Vram4;
            for (u32 i = 0; i < 0x14000; i += 4)
            {
                memu_store32FromC(0x06000000 + i, *(vu32*)(0x06000000 + i));
            }
            break;
        }
        case 5:
        {
            mem_setVramFMapping(MEM_VRAM_FG_MAIN_BG_10000);
            setVramLoad345();
            memu_itcmStore8Table[6] = (void*)memu_store8Vram5;
            memu_store8Table[6] = (u32)memu_store8Vram5;
            memu_itcmStore16Table[6] = (void*)memu_store16Vram5;
            memu_store16Table[6] = (u32)memu_store16Vram5;
            memu_itcmStore32Table[6] = (void*)memu_store32Vram5;
            memu_store32Table[6] = (u32)memu_store32Vram5;
            for (u32 i = 0; i < 0x14000; i += 4)
            {
                memu_store32FromC(0x06000000 + i, *(vu32*)(0x06000000 + i));
            }
            break;
        }
    }
}

[[gnu::section(".itcm")]]
void emu_regDispCntStore(u16 newValue)
{
    u32 oldValue = *(u16*)&emu_ioRegisters[GBA_REG_OFFS_DISPCNT];
    *(u16*)&emu_ioRegisters[GBA_REG_OFFS_DISPCNT] = newValue;
    u32 dsDispCnt = 0x10000 | (newValue & ~0x7F);
    if (newValue & 0x40)
        dsDispCnt |= 0x10;
    if (newValue & 0x20)
        dsDispCnt |= 1 << 23;
    u32 newMode = newValue & 7;
    u32 oldMode = oldValue & 7;
    switch (newMode)
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            dsDispCnt |= 2;
            dsDispCnt &= ~(1 << 11);
            break;
        }
        case 2:
        {
            dsDispCnt |= 2;
            dsDispCnt &= ~(3 << 8);
            break;
        }
        case 3:
        {
            dsDispCnt |= 5;
            dsDispCnt &= ~(11 << 8);
            break;
        }
        case 4:
        {
            dsDispCnt |= 5;
            dsDispCnt &= ~(11 << 8);
            u32 priority = *(u16*)&emu_ioRegisters[GBA_REG_OFFS_BG2CNT] & 3;
            REG_BG2CNT = (0x5080 + ((newValue & (1 << 4)) << 6)) | priority;
            break;
        }
        case 5:
        {
            dsDispCnt |= 5;
            dsDispCnt &= ~(11 << 8);
            u32 priority = *(u16*)&emu_ioRegisters[GBA_REG_OFFS_BG2CNT] & 3;
            REG_BG2CNT = (0x5084 + ((newValue & (1 << 4)) << 6)) | priority;
            break;
        }
    }

    if (oldMode != newMode && (oldMode >= 3 || newMode >= 3))
    {
        displayModeChange(oldMode, newMode);
    }

    REG_DISPCNT = dsDispCnt;
}
