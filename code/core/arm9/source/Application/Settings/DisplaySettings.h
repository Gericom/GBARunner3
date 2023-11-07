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

    /// @brief Specifies whether centering and masking of the GBA screen should be enabled.
    ///        This requires display capture with double buffering and adds 1 frame of latency.
    bool16 enableCenterAndMask = true;

    /// @brief Horizontal centering offset to be used when enableCenterAndMask is true.
    u16 centerOffsetX = 8;

    /// @brief Vertical centering offset to be used when enableCenterAndMask is true.
    u16 centerOffsetY = 16;

    /// @brief Width of the visible screen area when enableCenterAndMask is true.
    u16 maskWidth = 240;

    /// @brief Height of the visible screen area when enableCenterAndMask is true.
    u16 maskHeight = 160;
};
