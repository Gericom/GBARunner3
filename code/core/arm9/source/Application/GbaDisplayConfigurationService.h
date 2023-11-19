#pragma once
#include "Application/Settings/DisplaySettings.h"

class GbaDisplayConfigurationService
{
    void SetupCaptureOam(const DisplaySettings& displaySettings);
    void SetupCenterAndMask(const DisplaySettings& displaySettings);
    void SetupGbaScreen(const DisplaySettings& displaySettings);
    void SetupColorCorrection(const DisplaySettings& displaySettings);
    void SetupGbaScreenBrightness(const DisplaySettings& displaySettings);
public:
    void ApplyDisplaySettings(const DisplaySettings& displaySettings);
};

extern GbaDisplayConfigurationService gGbaDisplayConfigurationService;
