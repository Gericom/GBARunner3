#pragma once

enum MpuRegion
{
    MPU_REGION_0,
    MPU_REGION_1,
    MPU_REGION_2,
    MPU_REGION_3,
    MPU_REGION_4,
    MPU_REGION_5,
    MPU_REGION_6,
    MPU_REGION_7
};

#ifdef __cplusplus
extern "C" {
#endif

extern void mpu_setRegionICacheEnable(MpuRegion region, bool enabled);

#ifdef __cplusplus
}
#endif
