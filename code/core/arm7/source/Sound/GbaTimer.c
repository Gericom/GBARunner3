#include "common.h"
#include "GbaTimer.h"

void gbat_initTimer(gbat_t* timer)
{
    timer->isStarted = 0;
    timer->curNrOverflows = 0;
    timer->reload = 0;
    timer->control = 0;
    timer->counter = 0;
    timer->preScaleCounter = 0;
}

void gbat_updateTimer(gbat_t* timer)
{
    if (!(timer->control & GBAT_CONTROL_ENABLED))
    {        
        timer->isStarted = 0;
        timer->curNrOverflows = 0;
        return;
    }

    if (!timer->isStarted)
    {
        timer->counter = timer->reload << 16;
        timer->isStarted = 1;
        timer->curNrOverflows = 0;
        timer->preScaleCounter = 0;
    }
    else
    {
        if (timer->control & GBAT_CONTROL_SLAVE)
           return;//todo: implement chaining

        u32 ticks = 33594931;
        switch(timer->control & GBAT_CONTROL_PRESCALE_MASK)
        {
            case GBAT_CONTROL_PRESCALE_64:
                ticks >>= 6;
                break;
            case GBAT_CONTROL_PRESCALE_256:
                ticks >>= 8;
                break;
            case GBAT_CONTROL_PRESCALE_1024:
                ticks >>= 10;
                break;
        }

        u64 counter = timer->counter + ticks;
        u32 reload = timer->reload << 16;
        u32 oldTop = 0;
        u32 newTop;
        while (true)
        {
            newTop = counter >> 32;
            if (oldTop == newTop)
                break;
            oldTop = newTop;
            counter += reload;
        }
        timer->counter = counter;
        timer->curNrOverflows = newTop;
    }
}
