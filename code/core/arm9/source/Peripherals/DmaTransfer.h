#pragma once

typedef struct
{
    u32 curSrc;
    u32 curDst;
    void* dmaFunction;
} dma_channel_t;

#define DMA_FLAG_HBLANK(channel)    (1 << (channel))

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

