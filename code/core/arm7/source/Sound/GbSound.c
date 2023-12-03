#include "common.h"
#include <string.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/sound/soundChannel.h>
#include <libtwl/timer/timer.h>
#include <libtwl/sio/sio.h>
#include "GbaIoRegOffsets.h"
#include "GbaSoundDefinitions.h"
#include "GbaSound7.h"
#include "GbEnvelope.h"
#include "GbSweep.h"
#include "GbSound.h"

#define GB_CHANNEL_1_HW_L        8
#define GB_CHANNEL_1_HW_R        9
#define GB_CHANNEL_2_HW_L       10
#define GB_CHANNEL_2_HW_R       11
#define GB_CHANNEL_3_HW_L_0      6
#define GB_CHANNEL_3_HW_R_0      7
#define GB_CHANNEL_3_HW_L_1     12
#define GB_CHANNEL_3_HW_R_1     13
#define GB_CHANNEL_4_HW_L       14
#define GB_CHANNEL_4_HW_R       15

#define SOUND_TIMER_SEQUENCER    3

//information sources:
//- http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
//- http://belogic.com/gba/
//- https://github.com/Drenn1/GameYob
//- https://problemkaputt.de/gbatek.htm
//- https://github.com/mgba-emu/mgba

[[gnu::aligned(4)]]
static const u8 sLfsr7Table[] =
{
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
    0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x80,
    0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80,
    0x7F, 0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,

    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
    0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x80,
    0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80,
    0x7F, 0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,

    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
    0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x80,
    0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80,
    0x7F, 0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,

    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80,
    0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x80,
    0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x80, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x7F, 0x80,
    0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x80,
    0x7F, 0x80, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x7F, 0x7F,
    0x80, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F
};

// typedef struct
// {
//     u16 reg_gb_nr10;
//     u16 reg_gb_nr11_12;
//     u32 reg_gb_nr13_14;

//     u16 reg_gb_nr21_22;
//     u16 padding;
//     u32 reg_gb_nr23_24;

//     u16 reg_gb_nr30;
//     u16 reg_gb_nr31_32;
//     u32 reg_gb_nr33_34;

//     u32 reg_gb_nr41_42;
//     u32 reg_gb_nr43_44;

//     u16 reg_gb_nr50_51;
//     u16 reg_gba_snd_mix;

//     u32 reg_gb_nr52;
// } sound_emu_work_t;

// static sound_emu_work_t sound_emu_work;

static int sFrameSeqStep;

static int sChannelLengthCounter[4];
static bool sChannelUseLen[4];

static int sChannelFreq[4];
static int sChannelVolume[4];
static int sChannelEnvDir[4];
static int sChannelEnvSweep[4];
static int sChannelVolumeTimer[4];

static int sChannel1Duty;
static int sChannel2Duty;

static int sChannel1SweepTime;
static int sChannel1SweepTimer;
static int sChannel1SweepDir;
static int sChannel1SweepAmount;

static bool sChannel3IsMode64;
static int sChannel3CurPlayBank;
static bool sChannel3IsEnabled;
[[gnu::aligned(4)]]
static s8 sChannel3WaveData[2][32];

static int sChannel4Div;
static bool sChannel4Is7Bit;
static int sChannel4ShiftFreq;

static gbs_sweep_t sChannel1Sweep;

static gbs_envelope_t sChannel1Env;
static gbs_envelope_t sChannel2Env;
static gbs_envelope_t sChannel4Env;

static int sChannelEnableL;
static int sChannelEnableR;
static int sMasterVolumeL;
static int sMasterVolumeR;
static int sMixVolume;

static int sChannelPlaying;

static bool sMasterEnable;

static int gbDutyToDs(int gbDuty)
{
    if (gbDuty == 0)
        return SOUNDCNT_DUTY_12_5; //12.5%
    else if (gbDuty == 1)
        return SOUNDCNT_DUTY_25_0; //25%
    else if (gbDuty == 2)
        return SOUNDCNT_DUTY_50_0; //50%
    else
        return SOUNDCNT_DUTY_75_0; //75%
}

static void updateChannelDuty(int channel)
{
    switch (channel)
    {
        case 0:
        {
            u32 duty = gbDutyToDs(sChannel1Duty);
            snd_setChannelDuty(GB_CHANNEL_1_HW_L, duty);
            snd_setChannelDuty(GB_CHANNEL_1_HW_R, duty);
            break;
        }
        case 1:
        {
            u32 duty = gbDutyToDs(sChannel2Duty);
            snd_setChannelDuty(GB_CHANNEL_2_HW_L, duty);
            snd_setChannelDuty(GB_CHANNEL_2_HW_R, duty);
            break;
        }
    }
}

static u16 calculateChannel12Timer(int channelFreq)
{
    return 16 * channelFreq - 32768;
}

static u16 calculateChannel3Timer(void)
{
    return 8 * sChannelFreq[2] - 16384;
}

static u16 calculateChannel4Timer(void)
{
    int div = sChannel4Div == 0 ? 4 : (sChannel4Div * 8);
    div <<= sChannel4ShiftFreq + 1;
    return div > 16384 ? 0 : (-4 * div);
}

static void updateChannelFreq(int channel)
{
    switch (channel)
    {
        case 0:
        {
            u16 timer = calculateChannel12Timer(sChannelFreq[0]);
            snd_setChannelTimer(GB_CHANNEL_1_HW_L, timer);
            snd_setChannelTimer(GB_CHANNEL_1_HW_R, timer);
            break;
        }
        case 1:
        {
            u16 timer = calculateChannel12Timer(sChannelFreq[1]);
            snd_setChannelTimer(GB_CHANNEL_2_HW_L, timer);
            snd_setChannelTimer(GB_CHANNEL_2_HW_R, timer);
            break;
        }
        case 2:
        {
            u16 timer = calculateChannel3Timer();
            snd_setChannelTimer(GB_CHANNEL_3_HW_L_0, timer);
            snd_setChannelTimer(GB_CHANNEL_3_HW_R_0, timer);
            snd_setChannelTimer(GB_CHANNEL_3_HW_L_1, timer);
            snd_setChannelTimer(GB_CHANNEL_3_HW_R_1, timer);
            break;
        }
        case 3:
        {
            u16 timer = calculateChannel4Timer();
            snd_setChannelTimer(GB_CHANNEL_4_HW_L, timer);
            snd_setChannelTimer(GB_CHANNEL_4_HW_R, timer);
            break;
        }
    }
}

static void calcChannelVolume(int channel, int* volumeL, int* shiftL, int* volumeR, int* shiftR)
{
    if (sChannelEnableL & (1 << channel))
    {
        int sl = SOUNDCNT_SHIFT_2; //always at least a division by 4
        int vl = sChannelVolume[channel] * (sMasterVolumeL + 1);
        if (sMixVolume == 0)
            sl = SOUNDCNT_SHIFT_4; //division by 16
        else if (sMixVolume == 1)
            vl >>= 1; //sadly one bit is lost here
        if (vl > 127)
            vl = 127;
        *volumeL = vl;
        *shiftL = sl;
    }
    else
    {
        *volumeL = 0;
        *shiftL = SOUNDCNT_SHIFT_4;
    }

    if (sChannelEnableR & (1 << channel))
    {
        int sr = SOUNDCNT_SHIFT_2; //always at least a division by 4
        int vr = sChannelVolume[channel] * (sMasterVolumeR + 1);
        if (sMixVolume == 0)
            sr = SOUNDCNT_SHIFT_4; //division by 16
        else if (sMixVolume == 1)
            vr >>= 1; //sadly one bit is lost here
        if (vr > 127)
            vr = 127;
        *volumeR = vr;
        *shiftR = sr;
    }
    else
    {
        *volumeR = 0;
        *shiftR = SOUNDCNT_SHIFT_4;
    }
}

static void updateChannelVolume(int channel)
{
    sChannelVolume[0] = sChannel1Env.curVolume;
    sChannelVolume[1] = sChannel2Env.curVolume;
    sChannelVolume[3] = sChannel4Env.curVolume;
    int volumeLeft, shiftLeft, volumeRight, shiftRight;
    calcChannelVolume(channel, &volumeLeft, &shiftLeft, &volumeRight, &shiftRight);
    switch (channel)
    {
        case 0:
        {
            snd_setChannelVolume(GB_CHANNEL_1_HW_L, volumeLeft);
            snd_setChannelShift(GB_CHANNEL_1_HW_L, shiftLeft);
            snd_setChannelVolume(GB_CHANNEL_1_HW_R, volumeRight);
            snd_setChannelShift(GB_CHANNEL_1_HW_R, shiftRight);
            break;
        }
        case 1:
        {
            snd_setChannelVolume(GB_CHANNEL_2_HW_L, volumeLeft);
            snd_setChannelShift(GB_CHANNEL_2_HW_L, shiftLeft);
            snd_setChannelVolume(GB_CHANNEL_2_HW_R, volumeRight);
            snd_setChannelShift(GB_CHANNEL_2_HW_R, shiftRight);
            break;
        }
        case 2:
        {
            if (sChannel3IsMode64 || sChannel3CurPlayBank == 0)
            {
                snd_setChannelVolume(GB_CHANNEL_3_HW_L_0, volumeLeft);
                snd_setChannelShift(GB_CHANNEL_3_HW_L_0, shiftLeft);
                snd_setChannelVolume(GB_CHANNEL_3_HW_R_0, volumeRight);
                snd_setChannelShift(GB_CHANNEL_3_HW_R_0, shiftRight);
                snd_setChannelVolume(GB_CHANNEL_3_HW_L_1, 0);
                snd_setChannelShift(GB_CHANNEL_3_HW_L_1, 0);
                snd_setChannelVolume(GB_CHANNEL_3_HW_R_1, 0);
                snd_setChannelShift(GB_CHANNEL_3_HW_R_1, 0);
            }
            else
            {
                snd_setChannelVolume(GB_CHANNEL_3_HW_L_0, 0);
                snd_setChannelShift(GB_CHANNEL_3_HW_L_0, 0);
                snd_setChannelVolume(GB_CHANNEL_3_HW_R_0, 0);
                snd_setChannelShift(GB_CHANNEL_3_HW_R_0, 0);
                snd_setChannelVolume(GB_CHANNEL_3_HW_L_1, volumeLeft);
                snd_setChannelShift(GB_CHANNEL_3_HW_L_1, shiftLeft);
                snd_setChannelVolume(GB_CHANNEL_3_HW_R_1, volumeRight);
                snd_setChannelShift(GB_CHANNEL_3_HW_R_1, shiftRight);
            }
            break;
        }
        case 3:
        {
            snd_setChannelVolume(GB_CHANNEL_4_HW_L, volumeLeft);
            snd_setChannelShift(GB_CHANNEL_4_HW_L, shiftLeft);
            snd_setChannelVolume(GB_CHANNEL_4_HW_R, volumeRight);
            snd_setChannelShift(GB_CHANNEL_4_HW_R, shiftRight);
            break;
        }
    }
}

static void startChannel(int channel)
{
    sChannelVolume[0] = sChannel1Env.curVolume;
    sChannelVolume[1] = sChannel2Env.curVolume;
    sChannelVolume[3] = sChannel4Env.curVolume;
    int volumeLeft, shiftLeft, volumeRight, shiftRight, duty;
    calcChannelVolume(channel, &volumeLeft, &shiftLeft, &volumeRight, &shiftRight);
    switch (channel)
    {
        case 0:
        {
            duty = gbDutyToDs(sChannel1Duty);
            REG_SOUNDxCNT(GB_CHANNEL_1_HW_L) =
                SOUNDCNT_ENABLED |
                SOUNDCNT_FORMAT_PSG |
                SOUNDCNT_MODE_LOOP |
                duty |
                SOUNDCNT_PAN(0) |
                shiftLeft |
                SOUNDCNT_VOLUME(volumeLeft);
            REG_SOUNDxCNT(GB_CHANNEL_1_HW_R) =
                SOUNDCNT_ENABLED |
                SOUNDCNT_FORMAT_PSG |
                SOUNDCNT_MODE_LOOP |
                duty |
                SOUNDCNT_PAN(127) |
                shiftRight |
                SOUNDCNT_VOLUME(volumeRight);
            break;
        }
        case 1:
        {
            duty = gbDutyToDs(sChannel2Duty);
            REG_SOUNDxCNT(GB_CHANNEL_2_HW_L) =
                SOUNDCNT_ENABLED |
                SOUNDCNT_FORMAT_PSG |
                SOUNDCNT_MODE_LOOP |
                duty |
                SOUNDCNT_PAN(0) |
                shiftLeft |
                SOUNDCNT_VOLUME(volumeLeft);
            REG_SOUNDxCNT(GB_CHANNEL_2_HW_R) =
                SOUNDCNT_ENABLED |
                SOUNDCNT_FORMAT_PSG |
                SOUNDCNT_MODE_LOOP |
                duty |
                SOUNDCNT_PAN(127) |
                shiftRight |
                SOUNDCNT_VOLUME(volumeRight);
            break;
        }
        case 2:
        {
            snd_stopChannel(GB_CHANNEL_3_HW_L_0);
            snd_stopChannel(GB_CHANNEL_3_HW_R_0);
            snd_stopChannel(GB_CHANNEL_3_HW_L_1);
            snd_stopChannel(GB_CHANNEL_3_HW_R_1);
            if (sChannel3IsMode64)
            {
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_L_0) = (u32)&sChannel3WaveData[0][0];
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_R_0) = (u32)&sChannel3WaveData[0][0];
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_L_0) = 64 >> 2;
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_R_0) = 64 >> 2;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_L_0) = 0;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_R_0) = 0;
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_L_0) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(0) |
                    shiftLeft |
                    SOUNDCNT_VOLUME(volumeLeft);
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_R_0) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(127) |
                    shiftRight |
                    SOUNDCNT_VOLUME(volumeRight);
            }
            else
            {
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_L_0) = (u32)&sChannel3WaveData[0][0];
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_R_0) = (u32)&sChannel3WaveData[0][0];
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_L_1) = (u32)&sChannel3WaveData[1][0];
                REG_SOUNDxSAD(GB_CHANNEL_3_HW_R_1) = (u32)&sChannel3WaveData[1][0];
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_L_0) = 32 >> 2;
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_R_0) = 32 >> 2;
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_L_1) = 32 >> 2;
                REG_SOUNDxLEN(GB_CHANNEL_3_HW_R_1) = 32 >> 2;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_L_0) = 0;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_R_0) = 0;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_L_1) = 0;
                REG_SOUNDxPNT(GB_CHANNEL_3_HW_R_1) = 0;
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_L_0) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(0) |
                    shiftLeft |
                    SOUNDCNT_VOLUME(sChannel3CurPlayBank == 0 ? volumeLeft : 0);
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_R_0) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(127) |
                    shiftRight |
                    SOUNDCNT_VOLUME(sChannel3CurPlayBank == 0 ? volumeRight : 0);
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_L_1) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(0) |
                    shiftLeft |
                    SOUNDCNT_VOLUME(sChannel3CurPlayBank == 1 ? volumeLeft : 0);
                REG_SOUNDxCNT(GB_CHANNEL_3_HW_R_1) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(127) |
                    shiftRight |
                    SOUNDCNT_VOLUME(sChannel3CurPlayBank == 1 ? volumeRight : 0);
            }
            break;
        }
        case 3:
        {
            snd_stopChannel(GB_CHANNEL_4_HW_L);
            snd_stopChannel(GB_CHANNEL_4_HW_R);
            if (sChannel4Is7Bit)
            {
                REG_SOUNDxSAD(GB_CHANNEL_4_HW_L) = (u32)&sLfsr7Table[0];
                REG_SOUNDxSAD(GB_CHANNEL_4_HW_R) = (u32)&sLfsr7Table[0];
                REG_SOUNDxLEN(GB_CHANNEL_4_HW_L) = sizeof(sLfsr7Table) >> 2;
                REG_SOUNDxLEN(GB_CHANNEL_4_HW_R) = sizeof(sLfsr7Table) >> 2;
                REG_SOUNDxPNT(GB_CHANNEL_4_HW_L) = 0;
                REG_SOUNDxPNT(GB_CHANNEL_4_HW_R) = 0;
                REG_SOUNDxCNT(GB_CHANNEL_4_HW_L) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(0) |
                    shiftLeft |
                    SOUNDCNT_VOLUME(volumeLeft);
                REG_SOUNDxCNT(GB_CHANNEL_4_HW_R) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PCM8 |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(127) |
                    shiftRight |
                    SOUNDCNT_VOLUME(volumeRight);
            }
            else
            {
                REG_SOUNDxCNT(GB_CHANNEL_4_HW_L) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PSG |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(0) |
                    shiftLeft |
                    SOUNDCNT_VOLUME(volumeLeft);
                REG_SOUNDxCNT(GB_CHANNEL_4_HW_R) =
                    SOUNDCNT_ENABLED |
                    SOUNDCNT_FORMAT_PSG |
                    SOUNDCNT_MODE_LOOP |
                    SOUNDCNT_PAN(127) |
                    shiftRight |
                    SOUNDCNT_VOLUME(volumeRight);
            }
            break;
        }
    }
    sChannelPlaying |= 1 << channel;
    gSoundSharedData->soundCntX = sChannelPlaying;
}

static void stopChannel(int channel)
{
    switch (channel)
    {
        case 0:
            snd_stopChannel(GB_CHANNEL_1_HW_L);
            snd_stopChannel(GB_CHANNEL_1_HW_R);
            break;
        case 1:
            snd_stopChannel(GB_CHANNEL_2_HW_L);
            snd_stopChannel(GB_CHANNEL_2_HW_R);
            break;
        case 2:
            snd_stopChannel(GB_CHANNEL_3_HW_L_0);
            snd_stopChannel(GB_CHANNEL_3_HW_R_0);
            snd_stopChannel(GB_CHANNEL_3_HW_L_1);
            snd_stopChannel(GB_CHANNEL_3_HW_R_1);
            break;
        case 3:
            snd_stopChannel(GB_CHANNEL_4_HW_L);
            snd_stopChannel(GB_CHANNEL_4_HW_R);
            break;
    }
    sChannelPlaying &= ~(1 << channel);
    gSoundSharedData->soundCntX = sChannelPlaying;
}

/*
 * A length counter disables a channel when it decrements to zero. 
 * It contains an internal counter and enabled flag.
 * Writing a byte to NRx1 loads the counter with 64-data (256-data for wave channel). 
 * The counter can be reloaded at any time.
 *
 * A channel is said to be disabled when the internal enabled flag is clear. 
 * When a channel is disabled, its volume unit receives 0, otherwise its volume unit receives the output of the waveform generator.
 * Other units besides the length counter can enable/disable the channel as well.
 *
 * Each length counter is clocked at 256 Hz by the frame sequencer.
 * When clocked while enabled by NRx4 and the counter is not zero, it is decremented.
 * If it becomes zero, the channel is disabled.
 */
static void frameSeqUpdateLength()
{
    for (int i = 0; i < 4; i++)
    {
        if (sChannelUseLen[i] && sChannelLengthCounter[i])
        {
            if (--sChannelLengthCounter[i] == 0)
            {
                stopChannel(i);
            }
        }
    }
}

/*
 * A volume envelope has a volume counter and an internal timer clocked at 64 Hz by the frame sequencer.
 * When the timer generates a clock and the envelope period is not zero, a new volume is calculated by adding or subtracting (as set by NRx2) one from the current volume.
 * If this new volume within the 0 to 15 range, the volume is updated, otherwise it is left unchanged and no further automatic increments/decrements are made to the volume until the channel is triggered again.
 * 
 * When the waveform input is zero the envelope outputs zero, otherwise it outputs the current volume.
 * 
 * Writing to NRx2 causes obscure effects on the volume that differ on different Game Boy models (see obscure behavior).
 */
static void frameSeqUpdateVolume()
{
    if (sChannelPlaying & 1)
    {
        gbs_updateEnvelope(&sChannel1Env);
        updateChannelVolume(0);
    }
    if (sChannelPlaying & 2)
    {
        gbs_updateEnvelope(&sChannel2Env);
        updateChannelVolume(1);
    }
    if (sChannelPlaying & 8)
    {
        gbs_updateEnvelope(&sChannel4Env);
        updateChannelVolume(3);
    }
}

/*
 * The first square channel has a frequency sweep unit, controlled by NR10. 
 * This has a timer, internal enabled flag, and frequency shadow register. 
 * It can periodically adjust square 1's frequency up or down.
 * 
 * During a trigger event, several things occur:
 *		Square 1's frequency is copied to the shadow register.
 *		The sweep timer is reloaded.
 *		The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
 *		If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
 * Frequency calculation consists of taking the value in the frequency shadow register, shifting it right by sweep shift,
 * optionally negating the value, and summing this with the frequency shadow register to produce a new frequency.
 * What is done with this new frequency depends on the context.
 * 
 * The overflow check simply calculates the new frequency and if this is greater than 2047, square 1 is disabled.
 * 
 * The sweep timer is clocked at 128 Hz by the frame sequencer.
 * When it generates a clock and the sweep's internal enabled flag is set and the sweep period is not zero, a new frequency is calculated and the overflow check is performed.
 * If the new frequency is 2047 or less and the sweep shift is not zero, this new frequency is written back to the shadow frequency and square 1's frequency in NR13 and NR14,
 * then frequency calculation and overflow check are run AGAIN immediately using this new value, but this second new frequency is not written back.
 * 
 * Square 1's frequency can be modified via NR13 and NR14 while sweep is active, but the shadow frequency won't be affected so the next time the sweep updates the channel's frequency this modification will be lost.
 */
static void frameSeqUpdateSweep()
{
    // if (!(sChannelPlaying & 1) || sChannel1SweepTime == 0)
    // 	return;
    // if (++sChannel1SweepTimer == sChannel1SweepTime)
    // {
    // 	int delta = (sChannel1FreqShadow >> sChannel1SweepAmount) * sChannel1SweepDir;
    // 	if (sChannel1FreqShadow + delta >= 0 && sChannel1FreqShadow <= 2047)
    // 	{
    // 		sChannel1FreqShadow += delta;
    // 		sChannelFreq[0] = sChannel1FreqShadow;
    // 		updateChannelFreq(0);
    // 	}
    // 	else if (sChannel1FreqShadow + delta >= 2048)
    // 		stopChannel(0);
    // 	sChannel1SweepTimer = 0;
    // }
    if (sChannel1Sweep.enabled)
    {
        if (--sChannel1Sweep.step == 0)
        {
            if (!gbs_updateSweep(&sChannel1Sweep, 0, &sChannelFreq[0]))
                stopChannel(0);
            updateChannelFreq(0);
        }
    }
}

//The frame sequencer generates low frequency clocks for the modulation units. It is clocked by a 512 Hz timer.
void gbs_frameSeqTick()
{
    if (!sMasterEnable)
        return;

    // Step   Length Ctr  Vol Env     Sweep
    // ---------------------------------------
    // 0      Clock       -           -
    // 1      -           -           -
    // 2      Clock       -           Clock
    // 3      -           -           -
    // 4      Clock       -           -
    // 5      -           -           -
    // 6      Clock       -           Clock
    // 7      -           Clock       -
    // ---------------------------------------
    // Rate   256 Hz      64 Hz       128 Hz
    if ((sFrameSeqStep & 1) == 0)
        frameSeqUpdateLength();
    if ((sFrameSeqStep & 3) == 2)
        frameSeqUpdateSweep();
    if (sFrameSeqStep == 7)
        frameSeqUpdateVolume();

    sFrameSeqStep = (sFrameSeqStep + 1) & 7;
}

void gbs_init(void)
{
    sFrameSeqStep = 0;

    sChannelLengthCounter[0] = 0;
    sChannelLengthCounter[1] = 0;
    sChannelLengthCounter[2] = 0;
    sChannelLengthCounter[3] = 0;

    sChannelUseLen[0] = false;
    sChannelUseLen[1] = false;
    sChannelUseLen[2] = false;
    sChannelUseLen[3] = false;

    sChannelFreq[0] = 0;
    sChannelFreq[1] = 0;
    sChannelFreq[2] = 0;
    sChannelFreq[3] = 0;

    sChannelVolume[0] = 0;
    sChannelVolume[1] = 0;
    sChannelVolume[2] = 0;
    sChannelVolume[3] = 0;

    sChannelEnvDir[0] = 0;
    sChannelEnvDir[1] = 0;
    sChannelEnvDir[2] = 0;
    sChannelEnvDir[3] = 0;

    sChannelEnvSweep[0] = 0;
    sChannelEnvSweep[1] = 0;
    sChannelEnvSweep[2] = 0;
    sChannelEnvSweep[3] = 0;

    sChannelVolumeTimer[0] = 0;
    sChannelVolumeTimer[1] = 0;
    sChannelVolumeTimer[2] = 0;
    sChannelVolumeTimer[3] = 0;

    sChannel1Duty = 0;
    sChannel2Duty = 0;

    sChannel1SweepTime = 0;
    sChannel1SweepTimer = 0;
    sChannel1SweepDir = 0;
    sChannel1SweepAmount = 0;

    sChannel3IsMode64 = false;
    sChannel3CurPlayBank = 0;
    sChannel3IsEnabled = false;
    memset(sChannel3WaveData, 0, sizeof(sChannel3WaveData));

    sChannel4Div = 0;
    sChannel4Is7Bit = false;
    sChannel4ShiftFreq = 0;

    gbs_writeSweep(&sChannel1Sweep, 0);

    gbs_writeEnvelope(&sChannel1Env, 0);
    gbs_writeEnvelope(&sChannel2Env, 0);
    gbs_writeEnvelope(&sChannel4Env, 0);

    sChannelEnableL = 0;
    sChannelEnableR = 0;
    sMasterVolumeL = 0;
    sMasterVolumeR = 0;
    sMixVolume = 0;

    sChannelPlaying = 0;

    sMasterEnable = false;

    //setup timer 3 for the frame sequencer
    tmr_stop(SOUND_TIMER_SEQUENCER);
    tmr_configure(SOUND_TIMER_SEQUENCER, TMCNT_H_CLK_SYS, 0, true);

    u32 irq = RTOS_IRQ_TIMER(SOUND_TIMER_SEQUENCER);
    rtos_disableIrqMask(irq);
    rtos_setIrqFunc(irq, gbs_frameSeqTick);
    rtos_ackIrqMask(irq);
    rtos_enableIrqMask(irq);

    tmr_start(SOUND_TIMER_SEQUENCER);
}

//assumes an 8 bit write
void gbs_writeReg(u32 reg, u8 val)
{
    if (!sMasterEnable && reg != 0x84 && reg < 0x90)
        return;

    switch (reg)
    {
        case 0x60: //NR10
            if (!gbs_writeSweep(&sChannel1Sweep, val))
                stopChannel(0);
            // sChannel1SweepTime = (val >> 4) & 7;
            // sChannel1SweepDir = (val & 8) ? -1 : 1;
            // sChannel1SweepAmount = val & 7;
            break;
        case 0x62: //NR11
            sChannelLengthCounter[0] = 64 - (val & 0x3F);
            sChannel1Duty = val >> 6;
            updateChannelDuty(0);
            break;
        case 0x63: //NR12
            if (!gbs_writeEnvelope(&sChannel1Env, val))
                stopChannel(0);
            updateChannelVolume(0);
            break;
        case 0x64: //NR13
            sChannelFreq[0] = (sChannelFreq[0] & 0x700) | val;
            updateChannelFreq(0);
            break;
        case 0x65: //NR14
            sChannelFreq[0] = (sChannelFreq[0] & 0xFF) | ((val & 7) << 8);
            updateChannelFreq(0);
            sChannelUseLen[0] = (val >> 6) & 1;
            if (val & 0x80)
            {
                if (sChannelLengthCounter[0] == 0)
                    sChannelLengthCounter[0] = 64;
                sChannel1Sweep.realFreq = sChannelFreq[0];
                gbs_resetSweep(&sChannel1Sweep);
                bool playing = gbs_resetEnvelope(&sChannel1Env);
                if (playing && sChannel1Sweep.shift)
                    playing = gbs_updateSweep(&sChannel1Sweep, true, &sChannelFreq[0]);
                if (playing)
                    startChannel(0);
                else
                    stopChannel(0);
            }
            break;
        case 0x68: //NR21
            sChannelLengthCounter[1] = 64 - (val & 0x3F);
            sChannel2Duty = val >> 6;
            updateChannelDuty(1);
            break;
        case 0x69: //NR22
            if (!gbs_writeEnvelope(&sChannel2Env, val))
                stopChannel(1);
            updateChannelVolume(1);
            break;
        case 0x6C: //NR23
            sChannelFreq[1] = (sChannelFreq[1] & 0x700) | val;
            updateChannelFreq(1);
            break;
        case 0x6D: //NR24
            sChannelFreq[1] = (sChannelFreq[1] & 0xFF) | ((val & 7) << 8);
            updateChannelFreq(1);
            sChannelUseLen[1] = (val >> 6) & 1;
            if (val & 0x80)
            {
                if (sChannelLengthCounter[1] == 0)
                    sChannelLengthCounter[1] = 64;
                if (gbs_resetEnvelope(&sChannel2Env))
                    startChannel(1);
                else
                    stopChannel(1);
            }
            break;
        case 0x70: //NR30
            sChannel3IsMode64 = (val >> 5) & 1;
            sChannel3CurPlayBank = (val >> 6) & 1;
            sChannel3IsEnabled = (val >> 7) & 1;
            if (!sChannel3IsEnabled)
                stopChannel(2);
            else
                updateChannelVolume(2);
            break;
        case 0x72: //NR31
            sChannelLengthCounter[2] = 256 - val;
            break;
        case 0x73: //NR32
            if (val & 0x80)
                sChannelVolume[2] = 12; //75%
            else
            {
                int vol = (val >> 5) & 3;
                if (vol == 0)
                    sChannelVolume[2] = 0;
                else if (vol == 1)
                    sChannelVolume[2] = 16;
                else if (vol == 2)
                    sChannelVolume[2] = 8;
                else
                    sChannelVolume[2] = 4;
            }
            updateChannelVolume(2);
            break;
        case 0x74: //NR33
            sChannelFreq[2] = (sChannelFreq[2] & 0x700) | val;
            updateChannelFreq(2);
            break;
        case 0x75: //NR34
            sChannelFreq[2] = (sChannelFreq[2] & 0xFF) | ((val & 7) << 8);
            updateChannelFreq(2);
            sChannelUseLen[2] = (val >> 6) & 1;
            if (val & 0x80)
            {
                if (sChannelLengthCounter[2] == 0)
                    sChannelLengthCounter[2] = 256;
                if (sChannel3IsEnabled)
                {
                    startChannel(2);
                }
            }
            break;
        case 0x78: //NR41
            sChannelLengthCounter[3] = 64 - (val & 0x3F);
            break;
        case 0x79: //NR42
            if (!gbs_writeEnvelope(&sChannel4Env, val))
                stopChannel(3);
            updateChannelVolume(3);
            break;
        case 0x7C: //NR43
            sChannel4Div = val & 7;
            sChannel4Is7Bit = (val >> 3) & 1;
            sChannel4ShiftFreq = (val >> 4) & 0xF;
            updateChannelFreq(3);
            break;
        case 0x7D: //NR44
            sChannelUseLen[3] = (val >> 6) & 1;
            if (val & 0x80)
            {
                if (sChannelLengthCounter[3] == 0)
                    sChannelLengthCounter[3] = 64;
                if (gbs_resetEnvelope(&sChannel4Env))
                    startChannel(3);
                else
                    stopChannel(3);
            }
            break;
        case 0x80: //NR50
            sMasterVolumeL = (val >> 4) & 7;
            sMasterVolumeR = val & 7;
            updateChannelVolume(0);
            updateChannelVolume(1);
            updateChannelVolume(2);
            updateChannelVolume(3);
            break;
        case 0x81: //NR51
            sChannelEnableL = (val >> 4) & 0xF;
            sChannelEnableR = val & 0xF;
            updateChannelVolume(0);
            updateChannelVolume(1);
            updateChannelVolume(2);
            updateChannelVolume(3);
            break;

        case GBA_REG_OFFS_SOUNDCNT_H:
        {
            sMixVolume = val & GBA_SOUNDCNT_H_GB_VOLUME_MASK;
            updateChannelVolume(0);
            updateChannelVolume(1);
            updateChannelVolume(2);
            updateChannelVolume(3);
            break;
        }
        case 0x84: //NR52
            if (!(val & GBA_SOUNDCNT_X_MASTER_ENABLE))
            {
                sMasterEnable = false;
                //disable all sounds
                stopChannel(0);
                stopChannel(1);
                stopChannel(2);
                stopChannel(3);
            }
            else
            {
                sMasterEnable = true;
            }
            break;
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            sChannel3WaveData[1 - sChannel3CurPlayBank][(reg & 0xF) << 1] = 127 - ((val & 0xF0) | (val >> 4));
            sChannel3WaveData[1 - sChannel3CurPlayBank][((reg & 0xF) << 1) + 1] = 127 - (((val & 0xF) << 4) | (val & 0xF));
            break;
    }
}
