#pragma once

typedef struct
{
    u32 fifo[8];
    volatile u16 readOffset;
    volatile u16 writeOffset;

    volatile bool dmaRequest;
    u8 curPlaySampleCount;
    u32 curPlaySamples;
} gbas_direct_channel_t __attribute__((aligned(4)));

typedef struct
{
    gbas_direct_channel_t directChannels[2];
    bool masterEnable;
    u16 soundCntH;
} gbas_shared_t;
