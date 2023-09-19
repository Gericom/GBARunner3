#pragma once

typedef struct
{
    u32 fifo[8];
    volatile u16 readOffset;
    volatile u16 writeOffset;

    volatile bool dmaRequest;
    bool isInitial;
    s8 curSample;

    u32 curPlaySamples;
    u16 curPlaySampleCount;

    u16 timerIdx;
    u16 volume;
    u16 enables;

    u32 sampleCounter;
} gbas_direct_channel_t __attribute__((aligned(4)));

typedef struct
{
    gbas_direct_channel_t directChannels[2];
    bool masterEnable;
} gbas_shared_t;
