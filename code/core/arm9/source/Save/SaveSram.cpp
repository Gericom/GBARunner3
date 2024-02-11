#include "common.h"
#include "Core/Environment.h"
#include "Save.h"
#include "SaveSwi.h"
#include "SaveTypeInfo.h"
#include "SaveSram.h"

static const u32 sReadSramV110Sig[] = { 0xB0A7B590u, 0x6038466Fu, 0x60BA6079u, 0x49174817u };
static const u32 sReadSramV111Sig[] = { 0xB0A0B570u, 0x1C0D1C04u, 0x4A081C16u, 0x49088810u };

static const u32 sWriteSramV110Sig[] = { 0xB083B580u, 0x6038466Fu, 0x60BA6079u, 0x49094809u };
static const u32 sWriteSramV111Sig[] = { 0x1C05B530u, 0x1C131C0Cu, 0x88104A0Bu, 0x4008490Bu };

static const u32 sVerifySramV110Sig[] = { 0xB0B7B590u, 0x6038466Fu, 0x60BA6079u, 0x49174817u };
static const u32 sVerifySramV111Sig[] = { 0xB0B0B570u, 0x1C0D1C04u, 0x4A081C16u, 0x49088810u };

static void readSram(const u8* src, u8* dst, u32 size)
{
    for (u32 i = 0; i < size; i++)
        *dst++ = *src++;
}

static void writeSram(const u8* src, u8* dst, u32 size)
{
	if (Environment::IsDsiMode()) {
		for (u32 i = 0; i < size; i++) {
			*dst++ = *src++;
		}
		return;
	}
	bool writeToSav = false;
    for (u32 i = 0; i < size; i++) {
		if (*(dst + i) != *(src + i)) {
			writeToSav = true;
		}
        *(dst + i) = *(src + i);
	}
	if (!writeToSav) {
		return;
	}
	sav_writeSaveSizeToFileFromUserMode((u32)dst - 0x0E000000, size);
	sav_flushSaveFileFromUserMode();
}

static const u8* verifySram(const u8* src, const u8* tgt, u32 size)
{
    for (u32 i = 0; i < size; ++i)
    {
        u8 saveByte = *tgt++;
        u8 expectedByte = *src++;
        if (saveByte != expectedByte)
            return tgt - 1;
    }
    return nullptr;
}

bool sram_patchV110(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadSramV110Sig, 0, (void*)readSram)
        && sav_tryPatchFunction(sWriteSramV110Sig, 1, (void*)writeSram)
        && sav_tryPatchFunction(sVerifySramV110Sig, 2, (void*)verifySram);
}

bool sram_patchV111(const SaveTypeInfo* saveTypeInfo, FIL* romFile, u32 tagRomAddress, u8* tempBuffer)
{
    return sav_tryPatchFunction(sReadSramV111Sig, 0, (void*)readSram)
        && sav_tryPatchFunction(sWriteSramV111Sig, 1, (void*)writeSram)
        && sav_tryPatchFunction(sVerifySramV111Sig, 2, (void*)verifySram);
}
