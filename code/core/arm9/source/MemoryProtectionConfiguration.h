#pragma once
#include "MemoryProtectionUnit.h"

#define MPU_REGION_GBA_IWRAM    MPU_REGION_6
#define MPU_REGION_GBA_EWRAM    MPU_REGION_7

/// @brief Helper class for building memory protection regions.
class MemoryProtectionRegionBuilder
{
    u32 _regionConfiguration;
    bool _instructionCacheEnabled = false;
    bool _dataCacheEnabled = false;
    bool _regionBufferable = false;
    MpuAccessPermission _dataAccessPermission = MPU_ACCESS_PERMISSION_NONE;
    MpuAccessPermission _instructionAccessPermission = MPU_ACCESS_PERMISSION_NONE;

public:
    /// @brief Starts building a region with the given base address and size.
    /// @param regionBase The base address of the region. This should be size aligned.
    /// @param regionSize The size of the region.
    constexpr MemoryProtectionRegionBuilder(u32 regionBase, MpuRegionSize regionSize)
        : _regionConfiguration(regionBase | (static_cast<u32>(regionSize) << 1) | 1) { }

    /// @brief Enables instruction cache on the region.
    constexpr MemoryProtectionRegionBuilder& WithInstructionCache()
    {
        _instructionCacheEnabled = true;
        return *this;
    }

    /// @brief Enables data cache on the region.
    constexpr MemoryProtectionRegionBuilder& WithDataCache()
    {
        _dataCacheEnabled = true;
        return *this;
    }

    /// @brief Sets the bufferable bit of the region.
    constexpr MemoryProtectionRegionBuilder& Bufferable()
    {
        _regionBufferable = true;
        return *this;
    }

    /// @brief Sets the data access permission of the region.
    /// @param dataAccessPermission The data access permission.
    constexpr MemoryProtectionRegionBuilder& WithDataAccessPermission(MpuAccessPermission dataAccessPermission)
    {
        _dataAccessPermission = dataAccessPermission;
        return *this;
    }

    /// @brief Sets the instruction access permission of the region.
    /// @param dataAccessPermission The instruction access permission.
    constexpr MemoryProtectionRegionBuilder& WithInstructionAccessPermission(MpuAccessPermission instructionAccessPermission)
    {
        _instructionAccessPermission = instructionAccessPermission;
        return *this;
    }

    /// @brief Applies the region configuration to the given region.
    /// @param region The region to apply the configuration to.
    void ApplyToRegion(MpuRegion region) const
    {
        mpu_setRegionBaseAndSize(region, _regionConfiguration);
        mpu_setRegionDataAccessPermission(region, _dataAccessPermission);
        mpu_setRegionInstructionAccessPermission(region, _instructionAccessPermission);
        mpu_setRegionInstructionCacheEnable(region, _instructionCacheEnabled);
        mpu_setRegionDataCacheEnable(region, _dataCacheEnabled);
        mpu_setRegionDataBufferability(region, _regionBufferable);
    }
};
