#include "common.h"
#include <algorithm>
#include <array>
#include <span>
#include <ranges>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "MemCopy.h"
#include "VirtualMachine/VirtualMachine.h"

using namespace ::testing;

static const u8 sTestArray[] alignas(4)
{
    0x3C, 0xBC, 0x13, 0x11, 0x87, 0x1C, 0x60, 0xF0, 0xDD, 0xE6, 0xA6, 0x49,
    0x05, 0x41, 0x2D, 0xEF, 0xF8, 0xB3, 0xED, 0xCF, 0xC8, 0x15, 0x41, 0xE8,
    0x37, 0x8A, 0xA4, 0x9D, 0xE7, 0x0D, 0x14, 0x3D, 0xAA, 0x5F, 0xC5, 0xE5,
    0xB6, 0x9E, 0x60, 0xC0, 0x52, 0x61, 0xAE, 0x69, 0x09, 0x1F, 0x67, 0x03,
    0x3F, 0x36, 0x73, 0x99, 0x49, 0xFA, 0xED, 0x0F, 0xB7, 0xD2, 0xF5, 0xAA,
    0x52, 0x2E, 0x41, 0xCA
};

static u8 sTestOutputArray[64] alignas(4);

TEST(MemCopy16Tests, Src32AlignedDst32AlignedCountNot32AlignedCopiesDataProperly)
{
    // Arrange
    memset(sTestOutputArray, 0, sizeof(sTestOutputArray));

    // Act
    mem_copy16(&sTestArray[0], &sTestOutputArray[0], 62);

    // Assert
    auto output = std::vector<u16>((u16*)&sTestOutputArray[0], (u16*)&sTestOutputArray[62]);
    auto expected = std::vector<u16>((const u16*)&sTestArray[0], (const u16*)&sTestArray[62]);
    EXPECT_THAT(output, Pointwise(Eq(), expected));
}

TEST(MemCopy16Tests, Src16AlignedDst32AlignedCountNot32AlignedCopiesDataProperly)
{
    // Arrange
    memset(sTestOutputArray, 0, sizeof(sTestOutputArray));

    // Act
    mem_copy16(&sTestArray[2], &sTestOutputArray[0], 62);

    // Assert
    auto output = std::vector<u16>((u16*)&sTestOutputArray[0], (u16*)&sTestOutputArray[62]);
    auto expected = std::vector<u16>((const u16*)&sTestArray[2], (const u16*)&sTestArray[64]);
    EXPECT_THAT(output, Pointwise(Eq(), expected));
}

TEST(MemCopy16Tests, Src32AlignedDst16AlignedCountNot32AlignedCopiesDataProperly)
{
    // Arrange
    memset(sTestOutputArray, 0, sizeof(sTestOutputArray));

    // Act
    mem_copy16(&sTestArray[0], &sTestOutputArray[2], 62);

    // Assert
    auto output = std::vector<u16>((u16*)&sTestOutputArray[2], (u16*)&sTestOutputArray[64]);
    auto expected = std::vector<u16>((const u16*)&sTestArray[0], (const u16*)&sTestArray[62]);
    EXPECT_THAT(output, Pointwise(Eq(), expected));
}

TEST(MemCopy16Tests, Src16AlignedDst16AlignedCountNot32AlignedCopiesDataProperly)
{
    // Arrange
    memset(sTestOutputArray, 0, sizeof(sTestOutputArray));

    // Act
    mem_copy16(&sTestArray[2], &sTestOutputArray[2], 62);

    // Assert
    auto output = std::vector<u16>((u16*)&sTestOutputArray[2], (u16*)&sTestOutputArray[62]);
    auto expected = std::vector<u16>((const u16*)&sTestArray[2], (const u16*)&sTestArray[62]);
    EXPECT_THAT(output, Pointwise(Eq(), expected));
}
