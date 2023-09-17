#pragma once

#define SAVE_DATA_SIZE      (128 * 1024)

extern u8 gSaveData[SAVE_DATA_SIZE];

bool sav_tryPatchFunction(const u32* signature, u32 saveSwiNumber, void* patchFunction);
