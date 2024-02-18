#pragma once
#include "Fat/ff.h"
#include "SaveTypeInfo.h"
#include "GbaSaveShared.h"

#define SAVE_DATA_FILL              0xFF
#define SAVE_DATA_SIZE              (32 * 1024)
#define ISNITRO_SAVE_BUFFER         ((vu8*)0x02480000)
#define ISNITRO_SAVE_BUFFER_SIZE    (128 * 1024)

extern u8 gSaveData[SAVE_DATA_SIZE];
extern FIL gSaveFile;
extern gba_save_shared_t gGbaSaveShared;

#define SKIP_SAVE_CHECK_INSTRUCTION     0xE8BD9000 // pop {r12,pc}

extern u32 emu_vblankIrqSkipSaveCheckInstruction;

bool sav_tryPatchFunction(const u32* signature, u32 saveSwiNumber, void* patchFunction);
void sav_initializeSave(const SaveTypeInfo* saveTypeInfo, const char* savePath);

#ifdef __cplusplus
extern "C" {
#endif

u8 sav_readSaveByteFromFile(u32 offset);
void sav_writeSaveByteToFile(u32 offset, u8 data);
void sav_flushSaveFile(void);
void sav_writeSaveToFile(void);

#ifdef __cplusplus
}
#endif
