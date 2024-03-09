#include <nds.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/mem/memVram.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include "gbarunner9_bin.h"
#include "three.h"
#include "threeGlow.h"
#include "splashBg.h"
#include "runner.h"
#include "../../../core/common/IpcChannels.h"
#include "../../../core/common/FsIpcCommand.h"

#define HANDSHAKE_PART0     0xA
#define HANDSHAKE_PART1     0xB
#define HANDSHAKE_PART2     0xC

typedef void (*gbarunner9_func_t)(int argc, char* argv[]);

static fs_ipc_cmd_t sDldiIpcCommand;

extern u8 _dldi_start[16 * 1024];

static void initIpc()
{
    // We cannot use ipc_initFifoSystem here, because it tries to initialize the
    // arm9 retrieve part as well, and assumes the rtos irq system is used.
    
    ipc_clearSendFifo();
    ipc_ackFifoError();
    ipc_disableRecvFifoNotEmptyIrq();
    ipc_disableSendFifoEmptyIrq();
    ipc_enableFifo();

    while (ipc_getArm7SyncBits() != HANDSHAKE_PART0);
    ipc_setArm9SyncBits(HANDSHAKE_PART0);
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART1);
    ipc_setArm9SyncBits(HANDSHAKE_PART1);
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART2);
    ipc_setArm9SyncBits(HANDSHAKE_PART2);

    // wait for the arm7 to finish its initialization
    while (ipc_getArm7SyncBits() != 7);
}

static bool tryInitDldi()
{
    *(vu16*)0x04000204 |= 0x800; // DS cartridge slot to arm7
    DC_FlushRange(_dldi_start, sizeof(_dldi_start));
    sDldiIpcCommand.cmd = FS_IPC_CMD_DLDI_SETUP;
    sDldiIpcCommand.buffer = _dldi_start;
    DC_FlushRange(&sDldiIpcCommand, sizeof(sDldiIpcCommand));
    ipc_sendWordDirect(((((u32)&sDldiIpcCommand) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) | IPC_CHANNEL_FS);
    while (ipc_isRecvFifoEmpty());
    return ipc_recvWordDirect();
}

static void loadSplashScreen()
{
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    mem_setVramFMapping(MEM_VRAM_FG_MAIN_OBJ_00000);
    mem_setVramGMapping(MEM_VRAM_FG_MAIN_OBJ_04000);

    dmaCopy(splashBgTiles, (void*)0x06000000, splashBgTilesLen);
    dmaCopy(splashBgMap, (void*)0x06001000, splashBgMapLen);
    dmaCopy(splashBgPal, (void*)0x05000000, splashBgPalLen);
    dmaCopy(threeGlowTiles, (void*)0x06004000, threeGlowTilesLen);
    dmaCopy(threeGlowMap, (void*)0x06001800, threeGlowMapLen);
    dmaCopy(threeGlowPal, (void*)0x05000020, threeGlowPalLen);
    dmaCopy(threeTiles, (void*)0x06400000, threeTilesLen);
    dmaCopy(runnerTiles, (void*)(0x06400000 + threeTilesLen), runnerTilesLen);
    dmaCopy(threePal, (void*)0x05000200, threePalLen);
    dmaCopy(runnerPal, (void*)0x05000220, runnerPalLen);
}

int main(int argc, char* argv[])
{
    REG_IME = 0;
    rtos_disableIrqMask(~0u);
    rtos_ackIrqMask(~0u);

    initIpc();
    tryInitDldi();

    mem_setVramAMapping(MEM_VRAM_AB_LCDC);
    mem_setVramBMapping(MEM_VRAM_AB_LCDC);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);

    loadSplashScreen();

    DC_FlushAll();
    DC_InvalidateAll();
    IC_InvalidateAll();
    dmaCopyHalfWords(3, gbarunner9_bin, (void*)0x06800000, gbarunner9_bin_size);

    ((gbarunner9_func_t)0x06800000)(argc, argv);

    return 0;
}
