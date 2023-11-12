#pragma once

#define GBA_DMA_CONTROL_DST_STEP_SHIFT  5
#define GBA_DMA_CONTROL_SRC_STEP_SHIFT  7

#define GBA_DMA_CONTROL_REPEAT          (1 << 9)
#define GBA_DMA_CONTROL_32BIT           (1 << 10)
#define GBA_DMA_CONTROL_ROM_DREQ        (1 << 11)
#define GBA_DMA_CONTROL_IRQ             (1 << 14)
#define GBA_DMA_CONTROL_ENABLED         (1 << 15)

typedef struct
{
    u32 src;
    u32 dst;
    u16 count;
    u16 control;
} GbaDmaChannel;
