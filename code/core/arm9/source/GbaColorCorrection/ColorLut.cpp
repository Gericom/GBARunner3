#include "common.h"
#include <algorithm>
#include "ColorLut.h"
#include "GammaLut.h"

// Based on https://gist.github.com/profi200/bfa7be60b3eecb8c43f59000f626c743

u16 gColorLut[COLOR_LUT_SIZE];

// When the 2d engine converts from 5 to 6 bit, the lsb bit will always be zero (i.e. 31 -> 62)
static constexpr u32 rgb8ToRgb5(u32 value8)
{
    u32 value5 = (value8 * 63 + 255) / (255 * 2);
    if (value5 > 31)
    {
        return 31;
    }
    return value5;
}

// Convert RGB8 to RGB6 (for the 6-bit green)
static constexpr u32 rgb8ToRgb6(u32 value8)
{
    return (value8 * 63 + 128) / 255;
}

// Apply correction matrix from selected color profile
static void applyColorMatrix(const fix32<12> matrix[3][3],
                             fix32<12> r, fix32<12> g, fix32<12> b,
                             fix32<12>& outR, fix32<12>& outG, fix32<12>& outB)
{
    fix32<12> newR = (matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b);
    fix32<12> newG = (matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b);
    fix32<12> newB = (matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b);

    // We need to clamp specifically at this step or else color will get ruined
    outR = std::clamp(newR.Int(), 0, 255);
    outG = std::clamp(newG.Int(), 0, 255);
    outB = std::clamp(newB.Int(), 0, 255);
}

// Convert corrected RGB8 channels to RGB555 values (with the extra green bit)
static constexpr u16 packToRGB5(fix32<12> r, fix32<12> g, fix32<12> b)
{
    u16 r5 = rgb8ToRgb5(r.Int());
    u16 g6 = rgb8ToRgb6(g.Int()); // 6-bit green
    u16 b5 = rgb8ToRgb5(b.Int());

    return (b5 << 10) | ((g6 >> 1) << 5) | r5 | (g6 << 15); // bit 15 = extra green bit
}

// Main Function
static u16 applyColorCorrection(const u16 rgb5, const ColorProfile* preset, int gammaIndex)
{
    // Extract RGB channels
    int r5 = (rgb5 & 0x1F);
    int g5 = (rgb5 >> 5) & 0x1F;
    int b5 = (rgb5 >> 10) & 0x1F;

    // Scale from 5 bits to 8 bits (0–255)
    int r8 = (r5 * 255) / 31;
    int g8 = (g5 * 255) / 31;
    int b8 = (b5 * 255) / 31;

    // Convert to non-linear gamma (encode)
    fix32<12> rLin = gGammaLut.Encode(r8);
    fix32<12> gLin = gGammaLut.Encode(g8);
    fix32<12> bLin = gGammaLut.Encode(b8);

    // Apply luminance
    rLin = (rLin * preset->luminance);
    gLin = (gLin * preset->luminance);
    bLin = (bLin * preset->luminance);

    // Apply color correction matrix
    fix32<12> outR, outG, outB;
    applyColorMatrix(preset->matrix, rLin, gLin, bLin, outR, outG, outB);

    // Convert to linearized gamma (decode).
    outR = gGammaLut.Decode(outR.Int(), gammaIndex);
    outG = gGammaLut.Decode(outG.Int(), gammaIndex);
    outB = gGammaLut.Decode(outB.Int(), gammaIndex);

    // Denormalize and convert to RGB8.
    return packToRGB5(outR, outG, outB);
}

// Generate LUT using using the selected color profile and gamma index
void clut_initColorCorrection(const ColorProfile* preset, int gammaIndex)
{
    for (u32 i = 0; i < COLOR_LUT_SIZE; ++i)
    {
        gColorLut[i] = applyColorCorrection(i, preset, gammaIndex);
    }
}

void clut_disableColorCorrection()
{
    for (u32 i = 0; i < COLOR_LUT_SIZE; ++i)
    {
        gColorLut[i] = i;
    }
}
