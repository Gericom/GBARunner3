#include "common.h"
#include <string.h>
#include <libtwl/ipc/ipcSync.h>
#include "Fat/ff.h"
#include "Core/Environment.h"
#include "MemFastSearch.h"
#include "SaveSwi.h"
#include "SaveTypeInfo.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "MemoryEmulator/RomDefs.h"
#include "Save.h"
#include "Sha1.h"

[[gnu::section(".ewram.bss")]]
u8 gSaveData[SAVE_DATA_SIZE] alignas(32);

[[gnu::section(".ewram.bss")]]
FIL gSaveFile alignas(32);

static DWORD sClusterTable[64];
static u32 saveSize = 0;
static bool isSram;

static char saveHash[20];

bool sav_tryPatchFunction(const u32* signature, u32 saveSwiNumber, void* patchFunction)
{
    u32* function = (u32*)mem_fastSearch16((const u32*)ROM_LINEAR_DS_ADDRESS, ROM_LINEAR_SIZE, signature);
    if (!function)
        return false;

    sav_swiTable[saveSwiNumber] = patchFunction;
    *(u16*)function = SAVE_THUMB_SWI(saveSwiNumber);
    return true;
}

static void loadSaveClusterMap(void)
{
    sClusterTable[0] = sizeof(sClusterTable) / sizeof(DWORD);
    gSaveFile.cltbl = sClusterTable;
    f_lseek(&gSaveFile, CREATE_LINKMAP);
}

static void fillSaveFile(u32 start, u32 end)
{
    const u8 saveFill = SAVE_DATA_FILL;
    f_lseek(&gSaveFile, start);
    for (u32 i = start; i < end; ++i)
    {
        UINT written = 0;
        f_write(&gSaveFile, &saveFill, 1, &written);
    }
    f_sync(&gSaveFile);
}

void sav_initializeSave(const SaveTypeInfo* saveTypeInfo, const char* savePath)
{
    saveSize = saveTypeInfo ? saveTypeInfo->size : (32 * 1024);
	isSram = memcmp(saveTypeInfo->tag, "SRAM", 4) == 0;
    memset(gSaveData, SAVE_DATA_FILL, SAVE_DATA_SIZE);
    if (Environment::IsIsNitroEmulator())
    {
        memset((void*)ISNITRO_SAVE_BUFFER, SAVE_DATA_FILL, ISNITRO_SAVE_BUFFER_SIZE);
    }
    memset(&gSaveFile, 0, sizeof(gSaveFile));
    if (f_open(&gSaveFile, savePath, FA_OPEN_EXISTING | FA_READ | FA_WRITE) == FR_OK)
    {
        bool clusterMapLoaded = false;
        u32 initialSize = f_size(&gSaveFile);
        if (initialSize < saveSize)
        {
            if (f_lseek(&gSaveFile, saveSize) == FR_OK)
            {
                f_rewind(&gSaveFile);
                loadSaveClusterMap();
                clusterMapLoaded = true;
                fillSaveFile(initialSize, saveSize);
            }
        }

        if (!clusterMapLoaded)
        {
            loadSaveClusterMap();
            clusterMapLoaded = true;
        }

        if (saveSize <= SAVE_DATA_SIZE)
        {
            f_rewind(&gSaveFile);
            UINT read = 0;
            f_read(&gSaveFile, gSaveData, saveSize, &read);
			if (Environment::IsDsiMode() && isSram)
				SHA1(saveHash, (char*)gSaveData, saveSize);
        }

        if (Environment::IsIsNitroEmulator())
        {
            f_rewind(&gSaveFile);
            UINT read = 0;
            f_read(&gSaveFile, (void*)ISNITRO_SAVE_BUFFER, saveSize, &read);
        }
    }
    else if (!Environment::IsIsNitroEmulator())
    {
        if (f_open(&gSaveFile, savePath, FA_CREATE_NEW | FA_READ | FA_WRITE) == FR_OK)
        {
            if (f_lseek(&gSaveFile, saveSize) == FR_OK)
            {
                f_rewind(&gSaveFile);
                loadSaveClusterMap();
                fillSaveFile(0, saveSize);
            }
        }
    }
}

extern "C" u8 sav_readSaveByteFromFile(u32 saveAddress)
{
    vm_enableNestedIrqs();
    u8 saveByte;
    if (Environment::IsIsNitroEmulator())
    {
        // save buffer in extended memory
        saveByte = ISNITRO_SAVE_BUFFER[saveAddress];
    }
    else
    {
        // write to file
        f_lseek(&gSaveFile, saveAddress);
        UINT bytesRead = 0;
        f_read(&gSaveFile, &saveByte, 1, &bytesRead);
    }
    vm_disableNestedIrqs();
    return saveByte;
}

extern "C" void sav_writeSaveByteToFile(u32 saveAddress, u8 data)
{
    vm_enableNestedIrqs();
    if (Environment::IsIsNitroEmulator())
    {
        // save buffer in extended memory
        ISNITRO_SAVE_BUFFER[saveAddress] = data;
    }
    else
    {
        // write to file
        f_lseek(&gSaveFile, saveAddress);
        UINT bytesWritten = 0;
        f_write(&gSaveFile, &data, 1, &bytesWritten);
    }
    vm_disableNestedIrqs();
}

extern "C" void sav_writeSaveSizeToFile(u32 saveAddress, u32 size)
{
    vm_enableNestedIrqs();
    if (Environment::IsIsNitroEmulator())
    {
        // save buffer in extended memory
		for (u32 i = 0; i < size; i++)
		   ISNITRO_SAVE_BUFFER[saveAddress+i] = gSaveData[saveAddress+i];
    }
    else
    {
        // write to file
        f_lseek(&gSaveFile, saveAddress);
        UINT bytesWritten = 0;
        f_write(&gSaveFile, gSaveData+saveAddress, size, &bytesWritten);
    }
    vm_disableNestedIrqs();
}

extern "C" void sav_writeSaveToFile(void)
{
	if (!Environment::IsDsiMode() || !(ipc_getArm7SyncBits() == 6)) {
		return;
	}

	if (!isSram) {
		ipc_setArm9SyncBits(6); // notify arm7 to reboot or power off
        while (1);
	}

	char newSaveHash[20];
	SHA1(newSaveHash, (char*)gSaveData, saveSize);

	if (memcmp(newSaveHash, saveHash, 20) != 0) {
		// new save hash does not match the current hash, so proceed with writing
		if (Environment::IsIsNitroEmulator())
		{
			// save buffer in extended memory
			for (u32 i = 0; i < saveSize; i++)
			   ISNITRO_SAVE_BUFFER[i] = gSaveData[i];
		}
		else
		{
			// write to file
			f_lseek(&gSaveFile, 0);
			UINT bytesWritten = 0;
			f_write(&gSaveFile, gSaveData, saveSize, &bytesWritten);
		}
	}

	ipc_setArm9SyncBits(6); // notify arm7 to reboot or power off
    while (1);
}

extern "C" void sav_flushSaveFile(void)
{
    vm_enableNestedIrqs();
    if (!Environment::IsIsNitroEmulator())
    {
        f_sync(&gSaveFile);
    }
    vm_disableNestedIrqs();
}
