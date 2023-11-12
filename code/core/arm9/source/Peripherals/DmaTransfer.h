#pragma once

typedef struct
{
    u32 curSrc;
    u32 curDst;
} dma_channel_t;

#define DMA_FLAG_HBLANK(channel)    (1 << (channel))
#define DMA_FLAG_HBLANK_MASK        (DMA_FLAG_HBLANK(0) | DMA_FLAG_HBLANK(1) | DMA_FLAG_HBLANK(2) | DMA_FLAG_HBLANK(3))
#define DMA_FLAG_SOUND(channel)     (1 << (channel + 8))
#define DMA_FLAG_SOUND_MASK         (DMA_FLAG_SOUND(1) | DMA_FLAG_SOUND(2))

typedef struct
{
    u32 dmaFlags;
    dma_channel_t channels[4];
} dma_state_t;

extern dma_state_t dma_state;

#ifdef __cplusplus
extern "C" {
#endif

void dma_init(void);

#ifdef __cplusplus
}
#endif

