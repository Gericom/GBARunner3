#include "common.h"
#include <libtwl/ipc/ipcSync.h>
#include "GbaTimer.h"
#include "GbaSound.h"
#include "GbaSound7.h"

static gbas_shared_t* sSharedData;
static gbat_t sTimers[2];

static bool sPaused;

static void updateDirectChannel(gbas_direct_channel_t* channel)
{
    const gbat_t* timer = &sTimers[channel->timerIdx];
    //more than 2 implies a samplerate higher than 64kHz
    if (timer->curNrOverflows > 10)
        return;
    for (u32 i = 0; i < timer->curNrOverflows; ++i)
    {
        int fifoCount = channel->writeOffset - channel->readOffset;
        if (fifoCount < 0)
            fifoCount += 8;
        if (fifoCount <= 3 && !channel->dmaRequest)
        {
            channel->dmaRequest = true;
            ipc_triggerArm7Irq();
        }
        if (channel->curPlaySampleCount == 0)
        {
            if (fifoCount >= 1)
            {
                channel->curPlaySamples = channel->fifo[channel->readOffset];
                channel->readOffset = (channel->readOffset + 1) & 7;
                channel->curPlaySampleCount = 4;
            }
            else
            {
                u32 samp = (u8)channel->curSample;
                samp |= samp << 8;
                samp |= samp << 16;
                channel->curPlaySamples = samp;
                channel->curPlaySampleCount = 4;
                channel->readOffset = (channel->readOffset + 1) & 7;
            }            
        }

        channel->curSample = (s8)channel->curPlaySamples;
        channel->curPlaySamples >>= 8;
        channel->curPlaySampleCount--;
    }
}

static s16 applyBias(int sample)
{
    //todo: use real bias
    sample += 0x200;
    if (sample >= 0x400)
        sample = 0x3FF;
    else if (sample < 0)
        sample = 0;
    return (sample - 0x200) << 6;
}

void gbas_init(gbas_shared_t* sharedData)
{
    sSharedData = sharedData;
    gbat_initTimer(&sTimers[0]);
    gbat_initTimer(&sTimers[1]);
    sPaused = false; //true;
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

void gbas_updateMixer(s16* outLeft, s16* outRight)
{
    int left = 0, right = 0;

    if (!sPaused)
    {
        gbat_updateTimer(&sTimers[0]);
        gbat_updateTimer(&sTimers[1]);
        
        //master enable
        if (sSharedData->masterEnable)
        {
            updateDirectChannel(&sSharedData->directChannels[0]);
            updateDirectChannel(&sSharedData->directChannels[1]);

            // dmga_sample(sDmgSamp);

            // left = sDmgSamp[0];
            // right = sDmgSamp[1];

            int sampA = sSharedData->directChannels[0].curSample << 2;
            if (sSharedData->directChannels[0].volume == 0)
                sampA = sampA >> 1;

            if (sSharedData->directChannels[0].enables & 2)
                left += sampA;
            if (sSharedData->directChannels[0].enables & 1)
                right += sampA;

            int sampB = sSharedData->directChannels[1].curSample << 2;
            if (sSharedData->directChannels[1].volume == 0)
                sampB = sampB >> 1;

            if (sSharedData->directChannels[1].enables & 2)
                left += sampB;
            if (sSharedData->directChannels[1].enables & 1)
                right += sampB;
        }
    }

    *outLeft = applyBias(left);
    *outRight = applyBias(right);
}