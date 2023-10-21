#pragma once
#include "Enums/GbaScreen.h"
#include "Enums/GbaColorCorrection.h"

#define DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS_MIN  1
#define DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS_MAX  16

class DisplaySettings
{
public:
    /// @brief Specifies the DS screen to display the GBA game on.
    GbaScreen gbaScreen = GbaScreen::Top;

    /// @brief Specifies the type of color correction to use.
    GbaColorCorrection gbaColorCorrection = GbaColorCorrection::None;

    /// @brief Specifies the master brightness setting to use for the display the GBA game on.
    ///        Should be a value between 1 (darkest) and 16 (brightest).
    u16 gbaScreenBrightness = DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS_MAX;
};
