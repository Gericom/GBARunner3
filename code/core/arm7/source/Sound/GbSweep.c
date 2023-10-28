#include "common.h"
#include "GbSweep.h"

void gbs_resetSweep(gbs_sweep_t* sweep)
{
    sweep->step = sweep->time;
    sweep->enabled = sweep->step != 8 || sweep->shift;
    sweep->occurred = 0;
}

bool gbs_writeSweep(gbs_sweep_t* sweep, u16 val)
{
    sweep->shift = val & 7;
    u16 oldDir = sweep->direction;
    sweep->direction = (val >> 3) & 1;
    bool on = true;
    if (sweep->occurred && oldDir && !sweep->direction)
        on = false;
    sweep->occurred = 0;
    sweep->time = (val >> 4) & 7;
    if (!sweep->time)
        sweep->time = 8;
    return on;
}

bool gbs_updateSweep(gbs_sweep_t* sweep, bool initial, int* chanFreq)
{
    if (initial || sweep->time != 8)
    {
        u16 freq = sweep->realFreq;
        if (sweep->direction)
        {
            freq -= freq >> sweep->shift;
            if(!initial && freq >= 0)
            {
                sweep->realFreq = freq;
                *chanFreq = freq;
            }
        }
        else
        {
            freq += freq >> sweep->shift;
            if (freq < 2048)
            {
                if(!initial && sweep->shift)
                {
                    sweep->realFreq = freq;
                    *chanFreq = freq;
                    if(!gbs_updateSweep(sweep, true, chanFreq))
                        return false;
                }
            }
            else
                return false;
        }
        sweep->occurred = 1;
    }
    sweep->step = sweep->time;
    return true;
}