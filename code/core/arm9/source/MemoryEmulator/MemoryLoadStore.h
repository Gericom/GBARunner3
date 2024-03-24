#pragma once

extern u16 memu_load8Table[16];
extern u16 memu_load16Table[16];
extern u16 memu_load32Table[16];
extern u16 memu_store8Table[16];
extern u16 memu_store16Table[16];
extern u16 memu_store32Table[16];

typedef void (*memu_load_store_handler_t)();

extern memu_load_store_handler_t memu_load8WordTable[16];
extern memu_load_store_handler_t memu_load16WordTable[16];
extern memu_load_store_handler_t memu_load32WordTable[16];
extern memu_load_store_handler_t memu_store8WordTable[16];
extern memu_load_store_handler_t memu_store16WordTable[16];
extern memu_load_store_handler_t memu_store32WordTable[16];

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

static inline void memu_setLoad8Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_load8Table[region] = (u32)handler;
    memu_load8WordTable[region] = handler;
}

static inline void memu_setLoad16Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_load16Table[region] = (u32)handler;
    memu_load16WordTable[region] = handler;
}

static inline void memu_setLoad32Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_load32Table[region] = (u32)handler;
    memu_load32WordTable[region] = handler;
}

static inline void memu_setStore8Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_store8Table[region] = (u32)handler;
    memu_store8WordTable[region] = handler;
}

static inline void memu_setStore16Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_store16Table[region] = (u32)handler;
    memu_store16WordTable[region] = handler;
}

static inline void memu_setStore32Handler(u32 region, memu_load_store_handler_t handler)
{
    memu_store32Table[region] = (u32)handler;
    memu_store32WordTable[region] = handler;
}

#ifdef __cplusplus
}
#endif
