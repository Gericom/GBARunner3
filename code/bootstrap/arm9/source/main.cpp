#include <nds.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/mem/memVram.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include "gbarunner9_bin.h"

#define HANDSHAKE_PART0     0xA
#define HANDSHAKE_PART1     0xB
#define HANDSHAKE_PART2     0xC

typedef void (*gbarunner9_func_t)(void);

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

int main(int argc, char* argv[])
{
    REG_IME = 0;
    rtos_disableIrqMask(~0u);
    rtos_ackIrqMask(~0u);
    
    initIpc();

    mem_setVramAMapping(MEM_VRAM_AB_LCDC);
    mem_setVramBMapping(MEM_VRAM_AB_LCDC);

    DC_FlushAll();
    DC_InvalidateAll();
    IC_InvalidateAll();
    dmaCopyHalfWords(3, gbarunner9_bin, (void*)0x06800000, gbarunner9_bin_size);

    ((gbarunner9_func_t)0x06800000)();

    return 0;
}
