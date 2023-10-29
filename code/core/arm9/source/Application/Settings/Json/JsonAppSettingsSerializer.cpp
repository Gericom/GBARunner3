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
#define KEY_DISPLAY_SETTINGS_ENABLE_CENTER_AND_MASK "enableCenterAndMask"
#define KEY_DISPLAY_SETTINGS_CENTER_OFFSET_X        "centerOffsetX"
#define KEY_DISPLAY_SETTINGS_CENTER_OFFSET_Y        "centerOffsetY"
#define KEY_DISPLAY_SETTINGS_MASK_WIDTH             "maskWidth"
#define KEY_DISPLAY_SETTINGS_MASK_HEIGHT            "maskHeight"

#define KEY_RUN_SETTINGS                            "runSettings"
#define KEY_RUN_SETTINGS_JIT_PATCH_ADDRESSES        "jitPatchAddresses"
#define KEY_RUN_SETTINGS_ENABLE_WRAM_ICACHE         "enableWramICache"

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

    displaySettings.enableCenterAndMask
        = json[KEY_DISPLAY_SETTINGS_ENABLE_CENTER_AND_MASK] | static_cast<bool>(displaySettings.enableCenterAndMask);
    displaySettings.centerOffsetX
        = json[KEY_DISPLAY_SETTINGS_CENTER_OFFSET_X] | displaySettings.centerOffsetX;
    displaySettings.centerOffsetY
        = json[KEY_DISPLAY_SETTINGS_CENTER_OFFSET_Y] | displaySettings.centerOffsetY;
    displaySettings.maskWidth
        = json[KEY_DISPLAY_SETTINGS_MASK_WIDTH] | displaySettings.maskWidth;
    displaySettings.maskHeight
        = json[KEY_DISPLAY_SETTINGS_MASK_HEIGHT] | displaySettings.maskHeight;
}

static u32 parseHexString(const char* hexString)
{
    if (hexString == nullptr || hexString[0] == 0)
    {
        return 0;
    }

    if (hexString[0] == '0' && hexString[1] == 'x')
    {
        hexString += 2;
    }

    u32 value = 0;
    char c;
    while ((c = *hexString++) != 0)
    {
        // https://stackoverflow.com/a/57112610
        c = ((c & 0xF) + (c >> 6)) | ((c >> 3) & 8);
        value = (value << 4) | (c & 0xF);
    }

    return value;
}

static bool tryParseJitPatchAddresses(const JsonArrayConst& jitPatchAddresses, RunSettings& runSettings)
{
    if (jitPatchAddresses.isNull())
        return false;

    runSettings.jitPatchAddresses = std::make_unique<u32[]>(jitPatchAddresses.size());
    int i = 0;
    for (const char* addressString : jitPatchAddresses)
    {
        runSettings.jitPatchAddresses[i++] = parseHexString(addressString);
    }

    runSettings.jitPatchAddressCount = i;
    return true;
}

static void readRunSettings(const JsonObjectConst& json, RunSettings& runSettings)
{
    if (json.isNull())
        return;

    tryParseJitPatchAddresses(json[KEY_RUN_SETTINGS_JIT_PATCH_ADDRESSES], runSettings);
    runSettings.enableWramInstructionCache
        = json[KEY_RUN_SETTINGS_ENABLE_WRAM_ICACHE] | static_cast<bool>(runSettings.enableWramInstructionCache);
}

static void readJson(const JsonDocument& json, AppSettings& appSettings)
{
    readDisplaySettings(json[KEY_DISPLAY_SETTINGS], appSettings.displaySettings);
    readRunSettings(json[KEY_RUN_SETTINGS], appSettings.runSettings);
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
