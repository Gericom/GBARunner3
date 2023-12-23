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
#define KEY_DISPLAY_SETTINGS_BORDER_IMAGE           "borderImage"

#define KEY_RUN_SETTINGS                                    "runSettings"
#define KEY_RUN_SETTINGS_JIT_PATCH_ADDRESSES                "jitPatchAddresses"
#define KEY_RUN_SETTINGS_ENABLE_WRAM_ICACHE                 "enableWramICache"
#define KEY_RUN_SETTINGS_ENABLE_EWRAM_DCACHE                "enableEWramDCache"
#define KEY_RUN_SETTINGS_SELF_MODIFYING_PATCH_ADDRESSES     "selfModifyingPatchAddresses"

#define KEY_GAME_SETTINGS                           "gameSettings"
#define KEY_GAME_SETTINGS_SAVE_TYPE                 "saveType"

#define ENUM_STRING_GBA_SCREEN_TOP                  "top"
#define ENUM_STRING_GBA_SCREEN_BOTTOM               "bottom"

#define ENUM_STRING_GBA_COLOR_CORRECTION_NONE       "none"
#define ENUM_STRING_GBA_COLOR_CORRECTION_AGB_001    "agb001"

#define ENUM_STRING_GBA_BORDER_IMAGE_NONE           "none"
#define ENUM_STRING_GBA_BORDER_IMAGE_DEFAULT        "default"
#define ENUM_STRING_GBA_BORDER_IMAGE_GAME           "game"

#define ENUM_STRING_GBA_SAVE_TYPE_AUTO              "auto"
#define ENUM_STRING_GBA_SAVE_TYPE_NONE              "none"

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

static bool tryParseGbaBorderImage(const char* gbaBorderImageString, GbaBorderImage& gbaBorderImage)
{
    if (!gbaBorderImageString)
        return false;

    if (!strcasecmp(gbaBorderImageString, ENUM_STRING_GBA_BORDER_IMAGE_NONE))
        gbaBorderImage = GbaBorderImage::None;
    else if (!strcasecmp(gbaBorderImageString, ENUM_STRING_GBA_BORDER_IMAGE_DEFAULT))
        gbaBorderImage = GbaBorderImage::Default;
    else if (!strcasecmp(gbaBorderImageString, ENUM_STRING_GBA_BORDER_IMAGE_GAME))
        gbaBorderImage = GbaBorderImage::Game;
    else
        return false;

    return true;
}

static bool tryParseGbaSaveType(const char* gbaSaveTypeString, GbaSaveType& gbaSaveType)
{
    if (!gbaSaveTypeString)
        return false;

    if (!strcasecmp(gbaSaveTypeString, ENUM_STRING_GBA_SAVE_TYPE_AUTO))
        gbaSaveType = GbaSaveType::Auto;
    else if (!strcasecmp(gbaSaveTypeString, ENUM_STRING_GBA_SAVE_TYPE_NONE))
        gbaSaveType = GbaSaveType::None;
    else
        return false;

    return true;
}

static void readBoolSetting(const JsonVariantConst& jsonValue, bool16& setting)
{
    setting = jsonValue | static_cast<bool>(setting);
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

    readBoolSetting(json[KEY_DISPLAY_SETTINGS_ENABLE_CENTER_AND_MASK], displaySettings.enableCenterAndMask);
    displaySettings.centerOffsetX
        = json[KEY_DISPLAY_SETTINGS_CENTER_OFFSET_X] | displaySettings.centerOffsetX;
    displaySettings.centerOffsetY
        = json[KEY_DISPLAY_SETTINGS_CENTER_OFFSET_Y] | displaySettings.centerOffsetY;
    displaySettings.maskWidth
        = json[KEY_DISPLAY_SETTINGS_MASK_WIDTH] | displaySettings.maskWidth;
    displaySettings.maskHeight
        = json[KEY_DISPLAY_SETTINGS_MASK_HEIGHT] | displaySettings.maskHeight;
    tryParseGbaBorderImage(json[KEY_DISPLAY_SETTINGS_BORDER_IMAGE], displaySettings.borderImage);
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

static bool tryParseSelfModifyingPatchAddresses(const JsonArrayConst& selfModifyingPatchAddresses, RunSettings& runSettings)
{
    if (selfModifyingPatchAddresses.isNull())
        return false;

    runSettings.selfModifyingPatchAddresses = std::make_unique<u32[]>(selfModifyingPatchAddresses.size());
    int i = 0;
    for (const char* addressString : selfModifyingPatchAddresses)
    {
        runSettings.selfModifyingPatchAddresses[i++] = parseHexString(addressString);
    }

    runSettings.selfModifyingPatchAddressCount = i;
    return true;
}

static void readRunSettings(const JsonObjectConst& json, RunSettings& runSettings)
{
    if (json.isNull())
        return;

    tryParseJitPatchAddresses(json[KEY_RUN_SETTINGS_JIT_PATCH_ADDRESSES], runSettings);
    readBoolSetting(json[KEY_RUN_SETTINGS_ENABLE_WRAM_ICACHE], runSettings.enableWramInstructionCache);
    readBoolSetting(json[KEY_RUN_SETTINGS_ENABLE_EWRAM_DCACHE], runSettings.enableEWramDataCache);
    tryParseSelfModifyingPatchAddresses(json[KEY_RUN_SETTINGS_SELF_MODIFYING_PATCH_ADDRESSES], runSettings);
}

static void readGameSettings(const JsonObjectConst& json, GameSettings& gameSettings)
{
    if (json.isNull())
        return;

    tryParseGbaSaveType(json[KEY_GAME_SETTINGS_SAVE_TYPE], gameSettings.saveType);
}

static void readJson(const JsonDocument& json, AppSettings& appSettings)
{
    readDisplaySettings(json[KEY_DISPLAY_SETTINGS], appSettings.displaySettings);
    readRunSettings(json[KEY_RUN_SETTINGS], appSettings.runSettings);
    readGameSettings(json[KEY_GAME_SETTINGS], appSettings.gameSettings);
}

bool JsonAppSettingsSerializer::TryDeserialize(const TCHAR* filePath, AppSettings& appSettings)
{
    if (_settingsFile.Open(filePath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return false;

    u32 fileSize = _settingsFile.GetSize();
    if (fileSize == 0)
        return false;

    std::unique_ptr<u8[]> fileData(new(cache_align) u8[fileSize]);

    u32 bytesRead = 0;
    if (_settingsFile.Read(fileData.get(), fileSize, bytesRead) != FR_OK ||
        bytesRead != fileSize ||
        _settingsFile.Close() != FR_OK)
    {
        return false;
    }

    auto result = deserializeJson(_jsonDocument, fileData.get(), fileSize);
    if (result != DeserializationError::Ok)
    {
        gLogger->Log(LogLevel::Debug, "Json deserialization error: %d\n", result);
        return false;
    }

    readJson(_jsonDocument, appSettings);

    return true;
}
