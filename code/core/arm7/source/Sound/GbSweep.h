#pragma once

typedef struct
{
    u16 shift;
    u16 time;
    u16 step;
    u16 direction;
    u16 enabled;
    u16 occurred;
    u16 realFreq;
} gbs_sweep_t;

#ifdef __cplusplus
extern "C" {
#endif

void gbs_resetSweep(gbs_sweep_t* sweep);
bool gbs_writeSweep(gbs_sweep_t* sweep, u16 val);
bool gbs_updateSweep(gbs_sweep_t* sweep, bool initial, int* chanFreq);

#ifdef __cplusplus
}
#endif
