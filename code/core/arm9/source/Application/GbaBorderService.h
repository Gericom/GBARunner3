#pragma once
#include "common.h"
#include <memory>
#include "fat/File.h"
#include "Application/Settings/Enums/GbaBorderImage.h"

class GbaBorderService
{
public:
    void SetupBorder(GbaBorderImage borderImage, u32 gameCode) const;
    bool TryLoadBorderFromFile(const TCHAR* filePath) const;

private:
    bool CheckBmpHeader(const std::unique_ptr<File>& borderFile, u32& dataOffset) const;
    bool TryLoadPalette(const std::unique_ptr<File>& borderFile) const;
    bool TryLoadGraphics(const std::unique_ptr<File>& borderFile) const;
    bool TryLoadTopBottomGraphics(const std::unique_ptr<File>& borderFile, u8* tmpBuffer) const;
    bool TryLoadLeftAndRightGraphics(const std::unique_ptr<File>& borderFile, u8* tmpBuffer, u32& vramOffset) const;
    void CreateTopBottomTiles(const u8 *bitmapBuffer, u32 &vramOffset) const;
    void CreateTileMap(u8 *tileMap) const;
    void CreateOams() const;
    void CreateTopBottomOams(vu16 *&oamPtr, u32 tileIndex, u32 y) const;
    void CreateLeftRightOams(vu16*& oamPtr, u32 tileIndex, u32 x) const;
};

extern GbaBorderService gGbaBorderService;
