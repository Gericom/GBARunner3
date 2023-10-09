#include "common.h"
#include "AppSettingsService.h"

[[gnu::section(".ewram.bss")]]
static JsonAppSettingsSerializer sJsonAppSettingsSerializer;

AppSettingsService gAppSettingsService(&sJsonAppSettingsSerializer);

bool AppSettingsService::TryLoadAppSettings(const TCHAR* settingsFilePath)
{
    return _appSettingsSerializer->TryDeserialize(settingsFilePath, _appSettings);
}
