#pragma once
#include <array>

#define COLOR_LUT_SIZE      (1 << 15)

extern std::array<u16, COLOR_LUT_SIZE> gColorLut;

void clut_disableColorCorrection();
