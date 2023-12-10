#include "common.h"
#include <string.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfxBackground.h>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/gfx/gfxWindow.h>
#include <mini-printf.h>
#include "DsDefinitions.h"
#include "GbaDefinitions.h"
#include "MemCopy.h"
#include "GbaBorderService.h"

#define GAME_BORDER_FILE_PATH_FORMAT    "/_gba/borders/%c%c%c%c.bmp"
#define DEFAULT_BORDER_FILE_PATH        "/_gba/border.bmp"

#define BMP_SIGNATURE   0x4D42      // 'BM'
#define BI_RGB          0

GbaBorderService gGbaBorderService;

void GbaBorderService::SetupBorder(GbaBorderImage borderImage, u32 gameCode) const
{
    switch (borderImage)
    {
        case GbaBorderImage::Game:
        {
            auto path = std::make_unique<char[]>(128);
            mini_snprintf(path.get(), 128, GAME_BORDER_FILE_PATH_FORMAT,
                gameCode & 0xFF, (gameCode >> 8) & 0xFF,
                (gameCode >> 16) & 0xFF, gameCode >> 24);

            if (TryLoadBorderFromFile(path.get()))
            {
                gLogger->Log(LogLevel::Debug, "Loaded game specific border from %s\n", path.get());
                return;
            }

            // fall through
        }
        case GbaBorderImage::Default:
        {
            if (TryLoadBorderFromFile(DEFAULT_BORDER_FILE_PATH))
            {
                gLogger->Log(LogLevel::Debug, "Loaded default border\n");
                break;
            }

            // fall through
        }
        case GbaBorderImage::None:
        {
            // ensure frame is black
            GFX_PLTT_BG_SUB[0] = 0;
            GFX_PLTT_OBJ_SUB[0] = 0;
            break;
        }
    }
}

bool GbaBorderService::TryLoadBorderFromFile(const TCHAR* filePath) const
{
    auto borderFile = std::make_unique<File>();

    if (borderFile->Open(filePath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return false;

    u32 dataOffset = 0;
    if (!CheckBmpHeader(borderFile, dataOffset))
        return false;

    if (!TryLoadPalette(borderFile))
        return false;

    if (borderFile->Seek(dataOffset) != FR_OK)
        return false;

    return TryLoadGraphics(borderFile);
}

bool GbaBorderService::CheckBmpHeader(const std::unique_ptr<File>& borderFile, u32& dataOffset) const
{
    u16 signature;
    if (!borderFile->ReadExact(&signature, sizeof(u16)) || signature != BMP_SIGNATURE)
        return false;

    if (borderFile->Seek(0xA) != FR_OK)
        return false;

    if (!borderFile->ReadExact(&dataOffset, sizeof(u32)))
        return false;

    if (borderFile->Seek(0x12) != FR_OK)
        return false;

    u32 width = 0;
    if (!borderFile->ReadExact(&width, sizeof(u32)) || width != NDS_LCD_WIDTH)
        return false;

    if (borderFile->Seek(0x16) != FR_OK)
        return false;

    u32 height = 0;
    if (!borderFile->ReadExact(&height, sizeof(u32)) || height != NDS_LCD_HEIGHT)
        return false;

    if (borderFile->Seek(0x1C) != FR_OK)
        return false;

    u16 bitsPerPixel = 0;
    if (!borderFile->ReadExact(&bitsPerPixel, sizeof(u16)) || bitsPerPixel != 8)
        return false;

    u32 compression = 0;
    if (!borderFile->ReadExact(&compression, sizeof(u32)) || compression != BI_RGB)
        return false;

    return true;
}

bool GbaBorderService::TryLoadPalette(const std::unique_ptr<File>& borderFile) const
{
    if (borderFile->Seek(0x36) != FR_OK)
        return false;

    u32 color = 0;
    for (u32 i = 0; i < 256; i++)
    {
        if (!borderFile->ReadExact(&color, sizeof(u32)))
            return false;

        u32 b = color & 0xFF;
        u32 g = (color >> 8) & 0xFF;
        u32 r = (color >> 16) & 0xFF;

        // todo: improve the color conversion
        u32 rgb555 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
        GFX_PLTT_BG_SUB[i] = rgb555;
        GFX_PLTT_OBJ_SUB[i] = rgb555;
    }

    return true;
}


bool GbaBorderService::TryLoadGraphics(const std::unique_ptr<File>& borderFile) const
{
    auto tmpBuffer = std::make_unique_for_overwrite<u8[]>(NDS_LCD_WIDTH * 16);
    u32 vramOffset = 0;

    // bottom border
    if (!TryLoadTopBottomGraphics(borderFile, tmpBuffer.get()))
        return false;

    CreateTopBottomTiles(tmpBuffer.get(), vramOffset);

    // left and right border
    if (!TryLoadLeftAndRightGraphics(borderFile, tmpBuffer.get(), vramOffset))
        return false;

    // top border
    if (!TryLoadTopBottomGraphics(borderFile, tmpBuffer.get()))
        return false;

    CreateTopBottomTiles(tmpBuffer.get(), vramOffset);

    CreateTileMap(tmpBuffer.get());
    mem_copy32(tmpBuffer.get(), (void*)0x06844000, 1024);

    CreateOams();

    REG_BG2CNT_SUB = 0x4880;
    gfx_setSubBg2Affine(
        256, 0, 0, 256,
        0, 0);
    REG_DISPCNT_SUB |= (1 << 10);
    REG_WINOUT_SUB = (1 << 2) | (1 << 4);

    return true;
}

bool GbaBorderService::TryLoadTopBottomGraphics(const std::unique_ptr<File>& borderFile, u8* tmpBuffer) const
{
    for (u32 i = 0; i < 16; i++)
    {
        if (!borderFile->ReadExact(tmpBuffer + (15 - i) * NDS_LCD_WIDTH, NDS_LCD_WIDTH))
            return false;
    }

    return true;
}

bool GbaBorderService::TryLoadLeftAndRightGraphics(const std::unique_ptr<File>& borderFile,
    u8* tmpBuffer, u32& vramOffset) const
{
    for (u32 i = 0; i < GBA_LCD_HEIGHT; i++)
    {
        if (!borderFile->ReadExact(tmpBuffer + ((GBA_LCD_HEIGHT - 1) - i) * 8, 8) || // left
            borderFile->Seek(borderFile->GetOffset() + GBA_LCD_WIDTH) != FR_OK || // skip screen area
            !borderFile->ReadExact(tmpBuffer + (GBA_LCD_HEIGHT + (GBA_LCD_HEIGHT - 1) - i) * 8, 8)) // right
        {
            return false;
        }
    }

    mem_copy32(tmpBuffer, (void *)(0x06840000 + vramOffset), 2 * 8 * GBA_LCD_HEIGHT);
    mem_copy32(tmpBuffer, (void *)(0x06860000 + vramOffset), 2 * 8 * GBA_LCD_HEIGHT);
    vramOffset += 2 * 8 * GBA_LCD_HEIGHT;

    return true;
}

void GbaBorderService::CreateTopBottomTiles(const u8* bitmapBuffer, u32& vramOffset) const
{
    // 8x8 tiles in 32x16 pieces
    for (u32 pieceX = 0; pieceX < 256; pieceX += 32)
    {
        for (u32 pieceTileY = 0; pieceTileY < 16; pieceTileY += 8)
        {
            for (u32 pieceTileX = 0; pieceTileX < 32; pieceTileX += 8)
            {
                for (u32 tileY = 0; tileY < 8; tileY++)
                {
                    u32 x = pieceX + pieceTileX;
                    u32 y = pieceTileY + tileY;

                    auto tileRow = reinterpret_cast<const u32 *>(bitmapBuffer + y * 256 + x);
                    ((vu32 *)(0x06840000 + vramOffset))[0] = tileRow[0];
                    ((vu32 *)(0x06860000 + vramOffset))[0] = tileRow[0];
                    ((vu32 *)(0x06840000 + vramOffset))[1] = tileRow[1];
                    ((vu32 *)(0x06860000 + vramOffset))[1] = tileRow[1];
                    vramOffset += 8;
                }
            }
        }
    }
}

void GbaBorderService::CreateTileMap(u8* tileMap) const
{
    memset(tileMap, 0, 1024);

    u32 tileIndex = 0;
    for (u32 pieceX = 0; pieceX < (NDS_LCD_WIDTH / 8); pieceX += 4)
    {
        for (u32 pieceTileY = 0; pieceTileY < 2; pieceTileY++)
        {
            for (u32 pieceTileX = 0; pieceTileX < 4; pieceTileX++)
            {
                u32 x = pieceX + pieceTileX;

                // top
                tileMap[pieceTileY * 32 + x] = 104 + tileIndex;

                // bottom
                tileMap[(22 + pieceTileY) * 32 + x] = tileIndex;

                tileIndex++;
            }
        }
    }

    for (u32 i = 0; i < (GBA_LCD_HEIGHT / 8); i++)
    {
        // left
        tileMap[64 + 32 * i] = 64 + i;

        // right
        tileMap[64 + 31 + 32 * i] = 64 + (GBA_LCD_HEIGHT / 8) + i;
    }
}

void GbaBorderService::CreateOams() const
{
    vu16* oamPtr = GFX_OAM_SUB + 12 * 4;

    // bottom
    CreateTopBottomOams(oamPtr, 0, NDS_LCD_HEIGHT - 16);

    // left
    CreateLeftRightOams(oamPtr, 64, 0);

    // right
    CreateLeftRightOams(oamPtr, 84, NDS_LCD_WIDTH - 8);

    // top
    CreateTopBottomOams(oamPtr, 104, 0);
}

void GbaBorderService::CreateTopBottomOams(vu16*& oamPtr, u32 tileIndex, u32 y) const
{
    for (u32 x = 0; x < NDS_LCD_WIDTH; x += 32)
    {
        oamPtr[0] =
            GFX_OAM_ATTR0_SHAPE_32_16 |
            GFX_OAM_ATTR0_MODE_NORMAL |
            GFX_OAM_ATTR0_PLTT256 |
            GFX_OAM_ATTR0_Y(y);
        oamPtr[1] =
            GFX_OAM_ATTR1_SIZE_32_16 |
            GFX_OAM_ATTR1_X(x);
        oamPtr[2] =
            GFX_OAM_ATTR2_PRIORITY(0) |
            GFX_OAM_ATTR2_VRAM_OFFS(tileIndex << 1);

        oamPtr += 4;
        tileIndex += 8;
    }
}

void GbaBorderService::CreateLeftRightOams(vu16*& oamPtr, u32 tileIndex, u32 x) const
{
    for (u32 y = 0; y < GBA_LCD_HEIGHT; y += 32)
    {
        oamPtr[0] =
            GFX_OAM_ATTR0_SHAPE_8_32 |
            GFX_OAM_ATTR0_MODE_NORMAL |
            GFX_OAM_ATTR0_PLTT256 |
            GFX_OAM_ATTR0_Y(16 + y);
        oamPtr[1] =
            GFX_OAM_ATTR1_SIZE_8_32 |
            GFX_OAM_ATTR1_X(x);
        oamPtr[2] =
            GFX_OAM_ATTR2_PRIORITY(0) |
            GFX_OAM_ATTR2_VRAM_OFFS(tileIndex << 1);

        oamPtr += 4;
        tileIndex += 4;
    }
}
