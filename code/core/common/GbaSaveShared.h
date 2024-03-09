#pragma once

#define GBA_SAVE_STATE_CLEAN    0
#define GBA_SAVE_STATE_DIRTY    1
#define GBA_SAVE_STATE_WAIT     2
#define GBA_SAVE_STATE_WRITE    3

typedef struct
{
    volatile u8 saveState;
    u8* saveData;
    u32 saveDataSize;
} gba_save_shared_t;
