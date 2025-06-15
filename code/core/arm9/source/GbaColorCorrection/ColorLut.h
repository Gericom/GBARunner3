#pragma once
#include "ColorProfiles.h"

#define COLOR_LUT_SIZE      (1 << 15)

extern u16 gColorLut[COLOR_LUT_SIZE];

void clut_initColorCorrection(const ColorProfile* preset, int gammaIndex);
void clut_disableColorCorrection();
