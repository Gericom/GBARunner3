#pragma once

#include <nds/ndstypes.h>
#include "Core/Math/fixed.h"
#include "Application/Settings/Enums/GbaColorCorrection.h"

struct ColorProfile
{
    fix32<12> matrix[3][3]; // 3x3 correction matrix
    fix32<12> luminance;    // Luminance factor
};

const ColorProfile* cprof_getColorProfile(GbaColorCorrection colorProfile);
