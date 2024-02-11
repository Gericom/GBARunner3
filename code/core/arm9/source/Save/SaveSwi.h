#pragma once

#define SAVE_THUMB_SWI(x)   (0xDF80 + (x))

extern void* sav_swiTable[16];

#ifdef __cplusplus
extern "C" {
#endif

extern u32 sav_callSwi0();
extern u32 sav_callSwi1();
extern u32 sav_callSwi2();
extern u32 sav_callSwi3();
extern u32 sav_callSwi4();
extern u32 sav_callSwi5();
extern u32 sav_callSwi6();
extern u32 sav_callSwi7();
extern u32 sav_callSwi8();
extern u32 sav_callSwi9();
extern u32 sav_callSwi10();
extern u32 sav_callSwi11();
extern u32 sav_callSwi12();
extern u32 sav_callSwi13();
extern u32 sav_callSwi14();
extern u32 sav_callSwi15();

extern u8 sav_readSaveByte(u32 saveAddress);
extern void sav_writeSaveByte(u32 saveAddress, u8 value);

extern u8 sav_readSaveByteFromFileFromUserMode(u32 saveAddress);
extern void sav_writeSaveByteToFileFromUserMode(u32 saveAddress, u8 data);
extern void sav_writeSaveSizeToFileFromUserMode(u32 saveAddress, u32 size);
extern void sav_flushSaveFileFromUserMode(void);

#ifdef __cplusplus
}
#endif
