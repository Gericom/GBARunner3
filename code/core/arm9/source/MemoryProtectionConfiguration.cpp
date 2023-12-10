#include "common.h"
#include "MemoryEmulator/RomDefs.h"
#include "MemoryProtectionConfiguration.h"

extern "C" void setupMemoryProtection()
{
    // mpu region 0: ITCM, DTCM, uncached mmem, IO, GBA slot
    MemoryProtectionRegionBuilder(0x00000000, MPU_REGION_SIZE_4GB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_PRIV_READ_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_PRIV_READ_WRITE)
        .ApplyToRegion(MPU_REGION_0);

    // mpu region 1: Cached Main Memory
    MemoryProtectionRegionBuilder(0x02000000, MPU_REGION_SIZE_4MB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_PRIV_READ_WRITE)
        .WithDataCache()
        .ApplyToRegion(MPU_REGION_1);

    // mpu region 2: GBA VRAM
    MemoryProtectionRegionBuilder(0x06000000, MPU_REGION_SIZE_8MB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_PRIV_READ_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_PRIV_READ_WRITE)
        .Bufferable()
        .ApplyToRegion(MPU_REGION_2);

    // mpu region 3: Linear part of GBA rom
    MemoryProtectionRegionBuilder(ROM_LINEAR_DS_ADDRESS, MPU_REGION_SIZE_2MB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE)
        .WithDataCache()
        .WithInstructionCache()
        .ApplyToRegion(MPU_REGION_3);

    // mpu region 4: OBJ VRAM (DS)
    MemoryProtectionRegionBuilder(0x06400000, MPU_REGION_SIZE_32KB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE)
        .WithInstructionCache()
        .Bufferable()
        .ApplyToRegion(MPU_REGION_4);

    // mpu region 5: LCDC VRAM A and B
    MemoryProtectionRegionBuilder(0x06800000, MPU_REGION_SIZE_1MB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_USER_READ_PRIV_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_READ_WRITE)
        .WithDataCache()
        .WithInstructionCache()
        .Bufferable()
        .ApplyToRegion(MPU_REGION_5);

    // mpu region 6: GBA IWRAM
    MemoryProtectionRegionBuilder(0x03000000, MPU_REGION_SIZE_16MB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_READ_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_READ_WRITE)
        .WithInstructionCache()
        .ApplyToRegion(MPU_REGION_GBA_IWRAM);

    // mpu region 7: GBA EWRAM
    MemoryProtectionRegionBuilder(0x02000000, MPU_REGION_SIZE_256KB)
        .WithDataAccessPermission(MPU_ACCESS_PERMISSION_READ_WRITE)
        .WithInstructionAccessPermission(MPU_ACCESS_PERMISSION_READ_WRITE)
        .WithInstructionCache()
        .ApplyToRegion(MPU_REGION_GBA_EWRAM);
}
