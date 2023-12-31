#include "common.h"
#include <libtwl/ipc/ipcSync.h>
#include "GbaTimer.h"
#include "GbSound.h"
#include "GbaSound.h"
#include "GbaSoundDefinitions.h"
#include "GbaSound7.h"

gbas_shared_t* gSoundSharedData;
static gbat_t sTimers[2];
static gbas_direct_channel7_t sDirectChannels[2];
static u16 sSoundCntH;
static bool sPaused;

static s8 updateDirectChannel(gbas_direct_channel_t* channel, gbas_direct_channel7_t* channel7, u32 timerOverflows)
{
    u32 samples = channel7->curPlaySamples;
    // no overflows, or too many overflows
    if (timerOverflows == 0 || timerOverflows > 10)
    {
        return (s8)samples;
    }

    for (u32 i = timerOverflows; i > 0; --i)
    {
        u32 readOffset = channel->readOffset;
        u32 fifoCount = (channel->writeOffset - readOffset);
        if (!(fifoCount & 4) && !channel->dmaRequest)
        {
            channel->dmaRequest = true;
            ipc_triggerArm7Irq();
        }
        if (channel7->curPlaySampleCount != 0)
        {
            samples >>= 8;
            channel7->curPlaySampleCount--;
        }
        else if (__builtin_expect((fifoCount & 7) != 0, true))
        {
            samples = channel->fifo[readOffset];
            channel7->curPlaySampleCount = 3;
            channel->readOffset = (readOffset + 1) & 7;
        }
    }
    channel7->curPlaySamples = samples;

    return (s8)samples;
}

static inline s16 clampSample(int inSample)
{
    int outSample = inSample << 23;
    if (inSample != (outSample >> 23))
        outSample = 0x7FFFFFFF ^ (inSample >> 31);
    return outSample >> 16;
}

void gbas_init(gbas_shared_t* sharedData)
{
    gSoundSharedData = sharedData;
    gbat_initTimer(&sTimers[0]);
    gbat_initTimer(&sTimers[1]);
    gbs_init();
    sPaused = false; //true;
    sDirectChannels[0].curPlaySampleCount = 0;
    sDirectChannels[0].curPlaySamples = 0;
    sDirectChannels[1].curPlaySampleCount = 0;
    sDirectChannels[1].curPlaySamples = 0;
}

void gbas_setTimerReload(u32 timer, u16 reload)
{
    sTimers[timer].reload = reload;
}

void gbas_setTimerControl(u32 timer, u16 control)
{
    sTimers[timer].control = control;
    if (!(control & GBAT_CONTROL_ENABLED))
    {
        sTimers[timer].isStarted = false;
    }
}

void gbas_setSoundCntHLo(u8 value)
{
    ((u8*)&sSoundCntH)[0] = value;
}

void gbas_setSoundCntHHi(u8 value)
{
    ((u8*)&sSoundCntH)[1] = value & 0x77;
}

void gbas_updateMixer(s16* outLeft, s16* outRight)
{
    s16 finalLeft = 0;
    s16 finalRight = 0;

    if (!sPaused)
    {
        u32 timer0Overflows = gbat_updateTimer(&sTimers[0]);
        u32 timer1Overflows = gbat_updateTimer(&sTimers[1]);
        
        //master enable
        if (gSoundSharedData->masterEnable)
        {
            u32 soundCntH = sSoundCntH;
            int sampA = updateDirectChannel(&gSoundSharedData->directChannels[0],
                &sDirectChannels[0],
                (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_TIMER_1) ? timer1Overflows : timer0Overflows);
            int sampB = updateDirectChannel(&gSoundSharedData->directChannels[1],
                &sDirectChannels[1],
                (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_TIMER_1) ? timer1Overflows : timer0Overflows);

            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_VOLUME_FULL)
                sampA <<= 1;

            int left = 0;
            int right = 0;
            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_ENABLE_LEFT)
                left += sampA;
            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_ENABLE_RIGHT)
                right += sampA;

            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_VOLUME_FULL)
                sampB <<= 1;

            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_ENABLE_LEFT)
                left += sampB;
            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_ENABLE_RIGHT)
                right += sampB;

            finalLeft = clampSample(left);
            finalRight = clampSample(right);
        }
    }

    *outLeft = finalLeft;
    *outRight = finalRight;
}