#pragma once
#include "Fat/File.h"
#include "ArduinoJson.h"

class AppSettings;

#define JSON_RESERVED_SIZE  2048

class JsonAppSettingsSerializer
{
    File _settingsFile;
    StaticJsonDocument<JSON_RESERVED_SIZE> _jsonDocument;
public:
    bool TryDeserialize(const TCHAR* filePath, AppSettings& appSettings);
};
