#include "common.h"
#include <string.h>
#include <libtwl/rtos/rtosEvent.h>
#include <libtwl/rtos/rtosThread.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/timer/timer.h>
#include <libtwl/sound/soundChannel.h>
#include "GbaSound7.h"
#include "TimerIds.h"
#include "SoundThread.h"

#define SOUND_CHANNEL_LEFT              0
#define SOUND_CHANNEL_RIGHT             1
#define SOUND_CHANNEL_RELOAD            (-512)

#define SOUND_THREAD_PRIORITY           20
#define SOUND_TIMER_SAMPLERATE_RELOAD   (SOUND_CHANNEL_RELOAD * 2)
#define SOUND_BUFFER_SIZE               128

static rtos_event_t sSampleEvent;
static rtos_thread_t sSoundThread;

static u32 sSoundThreadStack[128];

static s16 sSoundBufferLeft[SOUND_BUFFER_SIZE];
static s16 sSoundBufferRight[SOUND_BUFFER_SIZE];

static int sSoundBufferWritePtr;
static u32 sSamplePosition;

static void soundTimerIrq(u32 irqMask)
{
    rtos_signalEvent(&sSampleEvent);
}

static void soundThreadMain(void* arg)
{
    while (true)
    {
        u16 playbackSampleCount = tmr_getCounter(TIMER_ID_SOUND_SAMPLE_COUNTER);
        int difference = (u16)(playbackSampleCount - (u16)(sSamplePosition / 4));
        difference *= 4;
        while (difference-- > 0)
        {
            gbas_updateMixer(&sSoundBufferLeft[sSoundBufferWritePtr], &sSoundBufferRight[sSoundBufferWritePtr]);
            sSoundBufferWritePtr = (sSoundBufferWritePtr + 1) % SOUND_BUFFER_SIZE;
            sSamplePosition++;
        }

        rtos_waitEvent(&sSampleEvent, false, true);
    }
}

static void clearSoundBuffers(void)
{
    memset(sSoundBufferLeft, 0, sizeof(sSoundBufferLeft));
    memset(sSoundBufferRight, 0, sizeof(sSoundBufferRight));
    sSamplePosition = 0;
    sSoundBufferWritePtr = SOUND_BUFFER_SIZE / 2;
}

static void createEvent(void)
{
    rtos_createEvent(&sSampleEvent);
}

static void startThread(void)
{
    rtos_createThread(&sSoundThread, SOUND_THREAD_PRIORITY, soundThreadMain,
        NULL, sSoundThreadStack, sizeof(sSoundThreadStack));
    rtos_wakeupThread(&sSoundThread);
}

static void startTimers(void)
{
    tmr_stop(TIMER_ID_SOUND_SAMPLERATE);
    tmr_stop(TIMER_ID_SOUND_SAMPLE_COUNTER);
    tmr_configure(TIMER_ID_SOUND_SAMPLERATE, TMCNT_H_CLK_SYS, SOUND_TIMER_SAMPLERATE_RELOAD * 4, true);
    tmr_configure(TIMER_ID_SOUND_SAMPLE_COUNTER, TMCNT_H_CLK_PREV_TMR_OVF, 0, false);

    u32 irq = RTOS_IRQ_TIMER(TIMER_ID_SOUND_SAMPLERATE);
    rtos_disableIrqMask(irq);
    rtos_setIrqFunc(irq, soundTimerIrq);
    rtos_ackIrqMask(irq);
    rtos_enableIrqMask(irq);

    tmr_start(TIMER_ID_SOUND_SAMPLE_COUNTER);
    tmr_start(TIMER_ID_SOUND_SAMPLERATE);
}

static void startSoundChannels(void)
{
    snd_stopChannel(SOUND_CHANNEL_LEFT);
    snd_stopChannel(SOUND_CHANNEL_RIGHT);
    snd_setChannelTimer(SOUND_CHANNEL_LEFT, SOUND_CHANNEL_RELOAD);
    snd_setChannelTimer(SOUND_CHANNEL_RIGHT, SOUND_CHANNEL_RELOAD);
    REG_SOUNDxSAD(SOUND_CHANNEL_LEFT) = (u32)sSoundBufferLeft;
    REG_SOUNDxPNT(SOUND_CHANNEL_LEFT) = 0;
    REG_SOUNDxLEN(SOUND_CHANNEL_LEFT) = sizeof(sSoundBufferLeft) >> 2;
    REG_SOUNDxCNT(SOUND_CHANNEL_LEFT) =
        SOUNDCNT_VOLUME(127) |
        SOUNDCNT_SHIFT_0 |
        SOUNDCNT_PAN(0) |
        SOUNDCNT_MODE_LOOP |
        SOUNDCNT_FORMAT_PCM16;
    REG_SOUNDxSAD(SOUND_CHANNEL_RIGHT) = (u32)sSoundBufferRight;
    REG_SOUNDxPNT(SOUND_CHANNEL_RIGHT) = 0;
    REG_SOUNDxLEN(SOUND_CHANNEL_RIGHT) = sizeof(sSoundBufferRight) >> 2;
    REG_SOUNDxCNT(SOUND_CHANNEL_RIGHT) =
        SOUNDCNT_VOLUME(127) |
        SOUNDCNT_SHIFT_0 |
        SOUNDCNT_PAN(127) |
        SOUNDCNT_MODE_LOOP |
        SOUNDCNT_FORMAT_PCM16;
    snd_startChannel(SOUND_CHANNEL_LEFT);
    snd_startChannel(SOUND_CHANNEL_RIGHT);
}

void sndt_start(void)
{
    clearSoundBuffers();
    createEvent();
    startThread();
    startTimers();
    startSoundChannels();
}
