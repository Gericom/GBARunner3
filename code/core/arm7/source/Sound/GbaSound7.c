#include "common.h"
#include <libtwl/ipc/ipcSync.h>
#include "GbaTimer.h"
#include "GbSound.h"
#include "GbaSound.h"
#include "GbaSoundDefinitions.h"
#include "GbaSound7.h"

gbas_shared_t* gSoundSharedData;
static gbat_t sTimers[2];

static bool sPaused;

static void updateDirectChannel(gbas_direct_channel_t* channel, const gbat_t* timer)
{
    //more than 2 implies a samplerate higher than 64kHz
    if (timer->curNrOverflows > 10)
        return;
    for (u32 i = 0; i < timer->curNrOverflows; ++i)
    {
        u32 readOffset = channel->readOffset;
        int fifoCount = channel->writeOffset - readOffset;
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
                channel->curPlaySamples = channel->fifo[readOffset];
            }
            else
            {
                u32 samp = (u8)channel->curPlaySamples;
                samp |= samp << 8;
                samp |= samp << 16;
                channel->curPlaySamples = samp;
            }            
            channel->curPlaySampleCount = 3;
            channel->readOffset = (readOffset + 1) & 7;
        }
        else
        {
            channel->curPlaySamples >>= 8;
            channel->curPlaySampleCount--;
        }        
    }
}

static int applyBias(int sample)
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
    gSoundSharedData = sharedData;
    gbat_initTimer(&sTimers[0]);
    gbat_initTimer(&sTimers[1]);
    gbs_init();
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
        if (gSoundSharedData->soundCntX & GBA_SOUNDCNT_X_MASTER_ENABLE)
        {
            u32 soundCntH = gSoundSharedData->soundCntH;
            updateDirectChannel(&gSoundSharedData->directChannels[0],
                &sTimers[(soundCntH & GBA_SOUNDCNT_H_DIRECT_A_TIMER_1) ? 1 : 0]);
            updateDirectChannel(&gSoundSharedData->directChannels[1],
                &sTimers[(soundCntH & GBA_SOUNDCNT_H_DIRECT_B_TIMER_1) ? 1 : 0]);

            // dmga_sample(sDmgSamp);

            // left = sDmgSamp[0];
            // right = sDmgSamp[1];

            int sampA = (s8)gSoundSharedData->directChannels[0].curPlaySamples << 2;
            if (!(soundCntH & GBA_SOUNDCNT_H_DIRECT_A_VOLUME_FULL))
                sampA = sampA >> 1;

            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_ENABLE_LEFT)
                left += sampA;
            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_A_ENABLE_RIGHT)
                right += sampA;

            int sampB = (s8)gSoundSharedData->directChannels[1].curPlaySamples << 2;
            if (!(soundCntH & GBA_SOUNDCNT_H_DIRECT_B_VOLUME_FULL))
                sampB = sampB >> 1;

            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_ENABLE_LEFT)
                left += sampB;
            if (soundCntH & GBA_SOUNDCNT_H_DIRECT_B_ENABLE_RIGHT)
                right += sampB;
        }
    }

    *outLeft = applyBias(left);
    *outRight = applyBias(right);
}