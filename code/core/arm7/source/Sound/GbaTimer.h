#pragma once

#define GBAT_CONTROL_PRESCALE_1       0
#define GBAT_CONTROL_PRESCALE_64      1
#define GBAT_CONTROL_PRESCALE_256     2
#define GBAT_CONTROL_PRESCALE_1024    3

#define GBAT_CONTROL_PRESCALE(x)      (((x) & 3) << 0)
#define GBAT_CONTROL_PRESCALE_MASK    GBAT_CONTROL_PRESCALE(3)

#define GBAT_CONTROL_SLAVE            (1 << 2)
#define GBAT_CONTROL_IRQ              (1 << 6)
#define GBAT_CONTROL_ENABLED          (1 << 7)

typedef struct
{
    bool isStarted;

    u16 reload;
    u16 control;

    u32 counter;
    u32 curNrOverflows; //number of overflows in the current 32kHz step
} gbat_t;

#ifdef __cplusplus
extern "C" {
#endif

void gbat_initTimer(gbat_t* timer);
void gbat_updateTimer(gbat_t* timer);

#ifdef __cplusplus
}
#endif
