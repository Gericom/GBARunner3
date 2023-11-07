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

enum MpuAccessPermission
{
    MPU_ACCESS_PERMISSION_NONE = 0b0000,
    MPU_ACCESS_PERMISSION_PRIV_READ_WRITE = 0b0001,
    MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE = 0b0010,
    MPU_ACCESS_PERMISSION_READ_WRITE = 0b0011,
    MPU_ACCESS_PERMISSION_PRIV_READ_ONLY = 0b0101,
    MPU_ACCESS_PERMISSION_READ_ONLY = 0b0110
};

enum MpuRegionSize
{
    MPU_REGION_SIZE_4KB = 11,
    MPU_REGION_SIZE_8KB = 12,
    MPU_REGION_SIZE_16KB = 13,
    MPU_REGION_SIZE_32KB = 14,
    MPU_REGION_SIZE_64KB = 15,
    MPU_REGION_SIZE_128KB = 16,
    MPU_REGION_SIZE_256KB = 17,
    MPU_REGION_SIZE_512KB = 18,
    MPU_REGION_SIZE_1MB = 19,
    MPU_REGION_SIZE_2MB = 20,
    MPU_REGION_SIZE_4MB = 21,
    MPU_REGION_SIZE_8MB = 22,
    MPU_REGION_SIZE_16MB = 23,
    MPU_REGION_SIZE_32MB = 24,
    MPU_REGION_SIZE_64MB = 25,
    MPU_REGION_SIZE_128MB = 26,
    MPU_REGION_SIZE_256MB = 27,
    MPU_REGION_SIZE_512MB = 28,
    MPU_REGION_SIZE_1GB = 29,
    MPU_REGION_SIZE_2GB = 30,
    MPU_REGION_SIZE_4GB = 31
};

#ifdef __cplusplus
extern "C" {
#endif

extern void mpu_setRegionInstructionCacheEnable(MpuRegion region, bool enabled);
extern void mpu_setRegionInstructionAccessPermission(MpuRegion region, MpuAccessPermission accessPermission);
extern void mpu_setRegionDataCacheEnable(MpuRegion region, bool enabled);
extern void mpu_setRegionDataAccessPermission(MpuRegion region, MpuAccessPermission accessPermission);
extern void mpu_setRegionDataBufferability(MpuRegion region, bool bufferable);
extern void mpu_setRegionBaseAndSize(MpuRegion region, u32 configuration);

#ifdef __cplusplus
}
#endif
