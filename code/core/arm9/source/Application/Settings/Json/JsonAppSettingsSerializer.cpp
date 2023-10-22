#include "common.h"
#include <libtwl/mem/memSwap.h>
#include <algorithm>
#include <memory>
#include "ArduinoJson.h"
#include "../AppSettings.h"
#include "JsonAppSettingsSerializer.h"

#pragma GCC optimize("Os")

#define KEY_DISPLAY_SETTINGS                        "displaySettings"
#define KEY_DISPLAY_SETTINGS_GBA_SCREEN             "gbaScreen"
#define KEY_DISPLAY_SETTINGS_GBA_COLOR_CORRECTION   "gbaColorCorrection"
#define KEY_DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS  "gbaScreenBrightness"

#define ENUM_STRING_GBA_SCREEN_TOP                  "top"
#define ENUM_STRING_GBA_SCREEN_BOTTOM               "bottom"

#define ENUM_STRING_GBA_COLOR_CORRECTION_NONE       "none"
#define ENUM_STRING_GBA_COLOR_CORRECTION_AGB_001    "agb001"

static bool tryParseGbaScreen(const char* gbaScreenString, GbaScreen& gbaScreen)
{
    if (!gbaScreenString)
        return false;

    if (!strcasecmp(gbaScreenString, ENUM_STRING_GBA_SCREEN_TOP))
        gbaScreen = GbaScreen::Top;
    else if (!strcasecmp(gbaScreenString, ENUM_STRING_GBA_SCREEN_BOTTOM))
        gbaScreen = GbaScreen::Bottom;
    else
        return false;

    return true;
}

static bool tryParseGbaColorCorrection(const char* gbaColorCorrectionString, GbaColorCorrection& gbaColorCorrection)
{
    if (!gbaColorCorrectionString)
        return false;

    if (!strcasecmp(gbaColorCorrectionString, ENUM_STRING_GBA_COLOR_CORRECTION_NONE))
        gbaColorCorrection = GbaColorCorrection::None;
    else if (!strcasecmp(gbaColorCorrectionString, ENUM_STRING_GBA_COLOR_CORRECTION_AGB_001))
        gbaColorCorrection = GbaColorCorrection::Agb001;
    else
        return false;

    return true;
}

static void readDisplaySettings(const JsonObjectConst& json, DisplaySettings& displaySettings)
{
    if (json.isNull())
        return;

    tryParseGbaScreen(json[KEY_DISPLAY_SETTINGS_GBA_SCREEN], displaySettings.gbaScreen);
    tryParseGbaColorCorrection(json[KEY_DISPLAY_SETTINGS_GBA_COLOR_CORRECTION], displaySettings.gbaColorCorrection);
    if (json[KEY_DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS].is<int>())
    {
        displaySettings.gbaScreenBrightness = std::clamp(json[KEY_DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS].as<int>(),
            DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS_MIN, DISPLAY_SETTINGS_GBA_SCREEN_BRIGHTNESS_MAX);
    }
}

static void readJson(const JsonDocument& json, AppSettings& appSettings)
{
    readDisplaySettings(json[KEY_DISPLAY_SETTINGS], appSettings.displaySettings);
}

bool JsonAppSettingsSerializer::TryDeserialize(const TCHAR* filePath, AppSettings& appSettings)
{
    if (_settingsFile.Open(filePath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return false;

    u32 fileSize = _settingsFile.GetSize();
    if (fileSize == 0)
        return false;

    std::unique_ptr<u8[]> fileData(new(cache_align) u8[fileSize]);
    u8* fileDataPtr = fileData.get();

    u32 bytesRead = 0;
    if (_settingsFile.Read(fileDataPtr, fileSize, bytesRead) != FR_OK ||
        bytesRead != fileSize ||
        _settingsFile.Close() != FR_OK)
    {
        return false;
    }

    auto result = deserializeJson(_jsonDocument, fileDataPtr, fileSize);
    if (result != DeserializationError::Ok)
    {
        gLogger->Log(LogLevel::Debug, "Json deserialization error: %d\n", result);
        return false;
    }

    readJson(_jsonDocument, appSettings);

    return true;
}
