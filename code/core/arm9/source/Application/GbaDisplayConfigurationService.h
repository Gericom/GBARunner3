#pragma once
#include "Application/Settings/DisplaySettings.h"

class GbaDisplayConfigurationService
{
    void SetupCaptureOam(const DisplaySettings& displaySettings);
    void SetupCaptureSprite(const DisplaySettings& displaySettings, vu16* oamPtr, int x, int y) const;
    void SetupCenterAndMask(const DisplaySettings& displaySettings);
    void SetupGbaScreen(const DisplaySettings& displaySettings);
    void SetupColorCorrection(const DisplaySettings& displaySettings);
    void SetupGbaScreenBrightness(const DisplaySettings& displaySettings);
public:
    void ApplyDisplaySettings(const DisplaySettings& displaySettings);
};

extern GbaDisplayConfigurationService gGbaDisplayConfigurationService;
