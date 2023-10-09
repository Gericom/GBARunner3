#pragma once
#include "Json/JsonAppSettingsSerializer.h"
#include "AppSettings.h"

class AppSettingsService
{
    JsonAppSettingsSerializer* _appSettingsSerializer;
    AppSettings _appSettings;
public:

    explicit AppSettingsService(JsonAppSettingsSerializer* appSettingsSerializer)
        : _appSettingsSerializer(appSettingsSerializer) { }

    bool TryLoadAppSettings(const TCHAR* settingsFilePath);

    AppSettings& GetAppSettings() { return _appSettings; }
    const AppSettings& GetAppSettings() const { return _appSettings; }
};

extern AppSettingsService gAppSettingsService;
