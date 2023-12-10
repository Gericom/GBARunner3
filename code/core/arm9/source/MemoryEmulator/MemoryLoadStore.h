#pragma once

extern void* memu_itcmLoad8Table[16];
extern void* memu_itcmLoad16Table[16];
extern void* memu_itcmLoad32Table[16];
extern void* memu_itcmStore8Table[16];
extern void* memu_itcmStore16Table[16];
extern void* memu_itcmStore32Table[16];

extern u16 memu_load8Table[16];
extern u16 memu_load16Table[16];
extern u16 memu_load32Table[16];
extern u16 memu_store8Table[16];
extern u16 memu_store16Table[16];
extern u16 memu_store32Table[16];

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Loads a 32-bit value from the given GBA memory address.
///        When unaligned rotation is applied.
/// @param address The address to load from.
/// @return The loaded value.
extern u32 memu_load32FromC(u32 address);

/// @brief Stores a 32-bit value to the given GBA memory address.
/// @param address The address to store to.
/// @param value The value to store.
extern void memu_store32FromC(u32 address, u32 value);

extern void memu_load8Undefined(void);
extern void memu_load16Undefined(void);
extern void memu_load32Undefined(void);
extern void memu_store8Undefined(void);
extern void memu_store16Undefined(void);
extern void memu_store32Undefined(void);

extern void memu_load8Vram012(void);
extern void memu_load16Vram012(void);
extern void memu_load32Vram012(void);
extern void memu_store8Vram012(void);
extern void memu_store16Vram012(void);
extern void memu_store32Vram012(void);

extern void memu_load8Vram345(void);
extern void memu_load16Vram345(void);
extern void memu_load32Vram345(void);

extern void memu_store8Vram3(void);
extern void memu_store16Vram3(void);
extern void memu_store32Vram3(void);

extern void memu_store8Vram4(void);
extern void memu_store16Vram4(void);
extern void memu_store32Vram4(void);

extern void memu_store8Vram5(void);
extern void memu_store16Vram5(void);
extern void memu_store32Vram5(void);

#ifdef __cplusplus
}
#endif
