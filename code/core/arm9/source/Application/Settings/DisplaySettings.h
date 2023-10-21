#pragma once
#include "Enums/GbaScreen.h"
#include "Enums/GbaColorCorrection.h"

class DisplaySettings
{
public:
    /// @brief Specifies the DS screen to display the GBA game on.
    GbaScreen gbaScreen = GbaScreen::Top;

    /// @brief Specifies the type of color correction to use.
    GbaColorCorrection gbaColorCorrection = GbaColorCorrection::None;
};
