#pragma once

#define GBA_DMA_CONTROL_DST_STEP_SHIFT      5
#define GBA_DMA_CONTROL_DST_STEP_MASK       3

#define GBA_DMA_CONTROL_DST_STEP_INCREMENT  0
#define GBA_DMA_CONTROL_DST_STEP_DECREMENT  1
#define GBA_DMA_CONTROL_DST_STEP_FIXED      2
#define GBA_DMA_CONTROL_DST_STEP_RELOAD     3

#define GBA_DMA_CONTROL_SRC_STEP_SHIFT      7
#define GBA_DMA_CONTROL_SRC_STEP_MASK       3

#define GBA_DMA_CONTROL_SRC_STEP_INCREMENT  0
#define GBA_DMA_CONTROL_SRC_STEP_DECREMENT  1
#define GBA_DMA_CONTROL_SRC_STEP_FIXED      2

#define GBA_DMA_CONTROL_MODE_SHIFT          12
#define GBA_DMA_CONTROL_MODE_MASK           3

#define GBA_DMA_CONTROL_MODE_IMMEDIATE      0
#define GBA_DMA_CONTROL_MODE_VBLANK         1
#define GBA_DMA_CONTROL_MODE_HBLANK         2
#define GBA_DMA_CONTROL_MODE_SPECIAL        3

#define GBA_DMA_CONTROL_REPEAT              (1 << 9)
#define GBA_DMA_CONTROL_32BIT               (1 << 10)
#define GBA_DMA_CONTROL_ROM_DREQ            (1 << 11)
#define GBA_DMA_CONTROL_IRQ                 (1 << 14)
#define GBA_DMA_CONTROL_ENABLED             (1 << 15)

typedef struct
{
    u32 src;
    u32 dst;
    u16 count;
    u16 control;
} GbaDmaChannel;
