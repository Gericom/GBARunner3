#pragma once

typedef enum
{
    GBA_SIO_DEVICE_NONE,
    GBA_SIO_DEVICE_RFU
} GbaSioDevice;

typedef struct
{
    vu32 gbaToSioData32;
    vu32 sioToGbaData32;
    volatile bool so;
    volatile bool si;
    volatile bool irqRequest;
} sio_shared_t;
