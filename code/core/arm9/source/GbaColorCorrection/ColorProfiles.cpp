#include "ColorLut.h"

// Libretro shader values. Credits: hunterk and Pokefan531.
// https://forums.libretro.com/t/real-gba-and-ds-phat-colors/1540/238
// Last updated 10-05-2025.

// const ColorProfile PRESET_NAME = 
//{
//    {
//        { [r], [gr], [br] },
//        { [rg], [g], [bg] },
//        { [rb], [gb], [b] }
//    },
//    luminance
//};

// libretro GBA AGB-001 color (sRGB).
static const ColorProfile sAgb001 =
{
    {
        { 0.905f, 0.195f, -0.1f },
        { 0.1f, 0.65f, 0.25f },
        { 0.1575f, 0.1425f, 0.7f }
    },
    0.91f
};

// libretro GBA SP (AGS-101) color (sRGB).
static const ColorProfile sAgs101 =
{
    {
        { 0.96f, 0.11f, -0.07f },
        { 0.0325f, 0.89f, 0.0775f },
        { 0.001f, -0.03f, 1.029f }
    },
    0.935f
};

// libretro GB micro OXY-001 color (sRGB).
static const ColorProfile sOxy001 =
{
    {
        { 0.8025f, 0.31f, -0.1125f },
        { 0.1f, 0.6875f, 0.2125f },
        { 0.1225f, 0.1125f, 0.765f }
    },
    0.9f
};

//// libretro NDS Phat NTR-001 color (sRGB).
static const ColorProfile sNtr001 =
{
    {
        { 0.835f, 0.27f, -0.105f },
        { 0.1f, 0.6375f, 0.2625f },
        { 0.105f, 0.175f, 0.72f }
    },
    0.905f
};

// libretro NDS lite USG-001 color (sRGB).
static const ColorProfile sUsg001 =
{
    {
        { 0.93f, 0.14f, -0.07f },
        { 0.025f, 0.9f, 0.075f },
        { 0.008f, -0.03f, 1.022f }
    },
    0.935f
};

// libretro PSP 1000 color (sRGB).
static const ColorProfile sPsp01g =
{
    {
        { 0.835f, 0.155f, -0.135f },
        { 0.0425f, 0.745f, 0.155f },
        { 0.0225f, 0.0145f, 0.963f }
    },
    0.99f
};

// libretro Nintendo Switch GBA Classics color (sRGB).
static const ColorProfile sNswIps =
{
    {
        { 0.865f, 0.1225f, 0.0125f },
        { 0.0575f, 0.925f, 0.0125f },
        { 0.0575f, 0.1225f, 0.82f }
    },
    1.f
};

// libretro Nintendo Switch GBA Classics Oled color (sRGB).
static const ColorProfile sNswOle =
{
    {
        { 1.54f, -0.48f, -0.06f },
        { -0.045f, 1.0625f, -0.0175f },
        { -0.0225f, -0.075f, 1.1025f }
    },
    1.f
};

// libretro Visual Boy Advance color.
static const ColorProfile sVbaEmu =
{
    {
        { 0.73f, 0.27f, 0.f },
        { 0.0825f, 0.6775f, 0.24f },
        { 0.0825f, 0.24f, 0.6775f }
    },
    1.f
};

// libretro No$GBA full color.
static const ColorProfile sNoCash =
{
    {
        { 0.62f, 0.1913f, -0.0917f },
        { 0.004f, 0.56f, 0.167f },
        { 0.004f, 0.167f, 0.56f },
    },
    0.949f
};

// libretro mGBA GBA shader color.
static const ColorProfile sMgba01 =
{
    {
        { 0.84f, 0.18f, 0.f },
        { 0.09f, 0.67f, 0.26f },
        { 0.15f, 0.10f, 0.73f }
    },
    0.99f
};

// This should exactly match "Application/Settings/Enums/GbaColorCorrection.h" if updated
static const ColorProfile* const sColorProfileLut[] =
{
    /* GbaColorCorrection::None     */ nullptr,
    /* GbaColorCorrection::Agb001   */ &sAgb001,
    /* GbaColorCorrection::Ags101   */ &sAgs101,
    /* GbaColorCorrection::Oxy001   */ &sOxy001,
    /* GbaColorCorrection::Ntr001   */ &sNtr001,
    /* GbaColorCorrection::Usg001   */ &sUsg001,
    /* GbaColorCorrection::Psp01g   */ &sPsp01g,
    /* GbaColorCorrection::NswIps   */ &sNswIps,
    /* GbaColorCorrection::NswOle   */ &sNswOle,
    /* GbaColorCorrection::VbaEmu   */ &sVbaEmu,
    /* GbaColorCorrection::NoCash   */ &sNoCash,
    /* GbaColorCorrection::mGba01   */ &sMgba01
};

const ColorProfile* cprof_getColorProfile(GbaColorCorrection colorProfile)
{
    return sColorProfileLut[static_cast<size_t>(colorProfile)];
}
