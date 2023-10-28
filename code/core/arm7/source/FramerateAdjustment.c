#include "common.h"
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/rtos/rtosIrq.h>
#include "GbaDefinitions.h"
#include "DsDefinitions.h"
#include "FramerateAdjustment.h"

#define FPS_ADJUST_LINE     203

static u32 sRateCounter;
static bool sFramerateAdjustmentActive;

static void vcountIrq(u32 irqMask)
{
    // each frame accumulate the difference in cycles
    sRateCounter += (GBA_CYCLES_PER_FRAME << 1) - NDS_CYCLES_PER_FRAME;

    // when behind by at least one scanline of cycles compensate by adjusting vcount
    // such that and extra scanline is inserted
    if(sRateCounter >= NDS_CYCLES_PER_LINE)
    {
        // apply the compensation to the counter
        sRateCounter -= NDS_CYCLES_PER_LINE;

        // safety check to prevent repeating two lines if the current line just happens to end
        while (gfx_isInHBlank()); 

        // repeat the current line
        gfx_setVCount(gfx_getVCount());
    }
}

void fps_initializeFramerateAdjustment(void)
{
    sFramerateAdjustmentActive = false;
    sRateCounter = 0;
}

void fps_startFramerateAdjustment(void)
{
    if (sFramerateAdjustmentActive)
    {
        return;
    }
    sFramerateAdjustmentActive = true;
    sRateCounter = 0;
    gfx_setVCountMatchIrqEnabled(false);
    rtos_disableIrqMask(RTOS_IRQ_VCOUNT);
    rtos_ackIrqMask(RTOS_IRQ_VCOUNT);
    rtos_setIrqFunc(RTOS_IRQ_VCOUNT, vcountIrq);
    rtos_enableIrqMask(RTOS_IRQ_VCOUNT);
    gfx_setVCountMatchLine(FPS_ADJUST_LINE);
    while (gfx_getVCount() <= FPS_ADJUST_LINE);
    gfx_setVCountMatchIrqEnabled(true);
}

void fps_stopFramerateAdjustment(void)
{
    if (!sFramerateAdjustmentActive)
    {
        return;
    }
    sFramerateAdjustmentActive = false;
    sRateCounter = 0;
    gfx_setVCountMatchIrqEnabled(false);
    rtos_disableIrqMask(RTOS_IRQ_VCOUNT);
    rtos_setIrqFunc(RTOS_IRQ_VCOUNT, NULL);
}
