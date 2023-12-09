#include "common.h"
#include <algorithm>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/gfx/gfxWindow.h>
#include <libtwl/gfx/gfxBackground.h>
#include <libtwl/sys/sysPower.h>
#include "DsDefinitions.h"
#include "ColorLut.h"
#include "SystemIpc.h"
#include "GbaDisplayConfigurationService.h"

GbaDisplayConfigurationService gGbaDisplayConfigurationService;

void GbaDisplayConfigurationService::SetupCaptureOam(const DisplaySettings& displaySettings)
{
    vu16* oamPtr = GFX_OAM_SUB;
    for (u32 y = 0; y < NDS_LCD_HEIGHT; y += 64)
    {
        for (u32 x = 0; x < NDS_LCD_WIDTH; x += 64)
        {
            oamPtr[0] =
                GFX_OAM_ATTR0_SHAPE_64_64 |
                GFX_OAM_ATTR0_MODE_BITMAP |
                GFX_OAM_ATTR0_Y(y + displaySettings.centerOffsetY);
            oamPtr[1] =
                GFX_OAM_ATTR1_SIZE_64_64 |
                GFX_OAM_ATTR1_X(x + displaySettings.centerOffsetX);
            oamPtr[2] =
                GFX_OAM_ATTR2_BMP_ALPHA(15) |
                GFX_OAM_ATTR2_PRIORITY(1) |
                GFX_OAM_ATTR2_VRAM_OFFS((((y + 64) >> 3) << 5) + (x >> 3));
            oamPtr += 4;
        }
    }
}

void GbaDisplayConfigurationService::SetupCenterAndMask(const DisplaySettings& displaySettings)
{
    u32 windowX0 = std::clamp<u32>(displaySettings.centerOffsetX, 0, NDS_LCD_WIDTH - 1);
    u32 windowY0 = std::clamp<u32>(displaySettings.centerOffsetY, 0, NDS_LCD_HEIGHT - 1);
    u32 windowX1 = std::clamp<u32>(windowX0 + displaySettings.maskWidth, 1, NDS_LCD_WIDTH);
    u32 windowY1 = std::clamp<u32>(windowY0 + displaySettings.maskHeight, 1, NDS_LCD_HEIGHT);
    if (windowX0 == 0 && windowX1 == NDS_LCD_WIDTH)
    {
        // use two windows to cover a width of 256
        gfx_setSubWindow0(0, windowY0, 255, windowY1);
        gfx_setSubWindow1(255, windowY0, 0, windowY1);
    }
    else
    {
        gfx_setSubWindow0(windowX0, windowY0, windowX1, windowY1);
        gfx_setSubWindow1(windowX0, windowY0, windowX1, windowY1);
    }

    REG_WININ_SUB = ((1 << 3) | (1 << 4)) | (((1 << 3) | (1 << 4)) << 8);
    REG_WINOUT_SUB = 0;
    REG_DISPCNT_SUB = 0x40017934;
    REG_BG3CNT_SUB = 0x4284;
    gfx_setSubBg3Affine(
        256, 0, 0, 256,
        -(displaySettings.centerOffsetX * 256),
        -(displaySettings.centerOffsetY * 256));

    SetupCaptureOam(displaySettings);
}

void GbaDisplayConfigurationService::SetupGbaScreen(const DisplaySettings& displaySettings)
{
    if (displaySettings.gbaScreen == GbaScreen::Top)
    {
        if (displaySettings.enableCenterAndMask)
            sys_setMainEngineToBottomScreen();
        else
            sys_setMainEngineToTopScreen();
        sysipc_setTopBacklight(true);
        sysipc_setBottomBacklight(false);
    }
    else
    {
        if (displaySettings.enableCenterAndMask)
            sys_setMainEngineToTopScreen();
        else
            sys_setMainEngineToBottomScreen();
        sysipc_setTopBacklight(false);
        sysipc_setBottomBacklight(true);
    }

    REG_DISPCAPCNT = 0x00360000;
    if (displaySettings.enableCenterAndMask)
    {
        SetupCenterAndMask(displaySettings);
    }
}

void GbaDisplayConfigurationService::SetupColorCorrection(const DisplaySettings& displaySettings)
{
    if (displaySettings.gbaColorCorrection == GbaColorCorrection::None)
    {
        clut_disableColorCorrection();
    }
}

void GbaDisplayConfigurationService::SetupGbaScreenBrightness(const DisplaySettings& displaySettings)
{
    if (displaySettings.enableCenterAndMask)
    {
        REG_MASTER_BRIGHT = 0x8000 | 16;
        REG_MASTER_BRIGHT_SUB = 0x8000 | (16 - displaySettings.gbaScreenBrightness);
    }
    else
    {
        REG_MASTER_BRIGHT = 0x8000 | (16 - displaySettings.gbaScreenBrightness);
        REG_MASTER_BRIGHT_SUB = 0x8000 | 16;
    }
}

void GbaDisplayConfigurationService::ApplyDisplaySettings(const DisplaySettings& displaySettings)
{
    SetupGbaScreen(displaySettings);
    SetupColorCorrection(displaySettings);
    SetupGbaScreenBrightness(displaySettings);
}
