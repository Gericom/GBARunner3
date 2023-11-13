#pragma once

typedef struct
{
    u32 curPlaySampleCount;
    u32 curPlaySamples;
} gbas_direct_channel7_t;

typedef struct
{
    u32 fifo[8];
    volatile u32 readOffset;
    volatile u32 writeOffset;

    volatile bool dmaRequest;
} gbas_direct_channel_t __attribute__((aligned(4)));

typedef struct
{
    u16 soundCntH;
    u8 soundCntX; // only enable flags
    u8 masterEnable;
    gbas_direct_channel_t directChannels[2];
} gbas_shared_t;
