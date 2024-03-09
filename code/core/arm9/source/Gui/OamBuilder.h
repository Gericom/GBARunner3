#pragma once
#include <libtwl/gfx/gfxOam.h>
#include "Core/Math/Point.h"

class OamBuilder
{
    u16 _attr0;
    u16 _attr1;
    u16 _attr2;

    OamBuilder()
        : _attr0(0), _attr1(0), _attr2(0) { }

public:
    template <int width, int height>
    static constexpr OamBuilder OamWithSize(const Point& position, u32 vramOffset)
    {
        return OamWithSize<width, height>(position.x, position.y, vramOffset);
    }

    template <int width, int height>
    static constexpr OamBuilder OamWithSize(int x, int y, u32 vramOffset)
    {
        OamBuilder builder;
        switch (width)
        {
            case 8:
            {
                switch (height)
                {
                    case 8:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_8_8;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_8_8;
                        break;
                    case 16:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_8_16;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_8_16;
                        break;
                    case 32:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_8_32;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_8_32;
                        break;
                    default:
                        static_assert(width != 8 || (height == 8 || height == 16 || height == 32), "Invalid oam height for width 8");
                        break;
                }
                break;
            }
            case 16:
            {
                switch (height)
                {
                    case 8:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_16_8;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_16_8;
                        break;
                    case 16:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_16_16;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_16_16;
                        break;
                    case 32:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_16_32;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_16_32;
                        break;
                    default:
                        static_assert(width != 16 || (height == 8 || height == 16 || height == 32), "Invalid oam height for width 16");
                        break;
                }
                break;
            }
            case 32:
            {
                switch (height)
                {
                    case 8:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_32_8;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_32_8;
                        break;
                    case 16:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_32_16;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_32_16;
                        break;
                    case 32:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_32_32;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_32_32;
                        break;
                    case 64:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_32_64;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_32_64;
                        break;
                    default:
                        static_assert(width != 32 || (height == 8 || height == 16 || height == 32 || height == 64), "Invalid oam height for width 32");
                        break;
                }
                break;
            }
            case 64:
            {
                switch (height)
                {
                    case 32:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_64_32;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_64_32;
                        break;
                    case 64:
                        builder._attr0 = GFX_OAM_ATTR0_SHAPE_64_64;
                        builder._attr1 = GFX_OAM_ATTR1_SIZE_64_64;
                        break;
                    default:
                        static_assert(width != 64 || (height == 32 || height == 64), "Invalid oam height for width 64");
                        break;
                }
                break;
            }
            default:
                static_assert(width == 8 || width == 16 || width == 32 || width == 64, "Invalid oam width");
                break;
        }
        builder._attr0 |= GFX_OAM_ATTR0_Y(y);
        builder._attr1 |= GFX_OAM_ATTR1_X(x);
        builder._attr2 = GFX_OAM_ATTR2_VRAM_OFFS(vramOffset);
        return builder;
    }

    constexpr OamBuilder& Hidden()
    {
        _attr0 &= ~GFX_OAM_ATTR0_DOUBLE_AFFINE;
        _attr0 |= GFX_OAM_ATTR0_HIDDEN;
        return *this;
    }

    constexpr OamBuilder& AsWindow()
    {
        _attr0 &= ~GFX_OAM_ATTR0_MODE_BITMAP;
        _attr0 |= GFX_OAM_ATTR0_MODE_WINDOW;
        return *this;
    }

    constexpr OamBuilder& AsTranslucent()
    {
        _attr0 &= ~GFX_OAM_ATTR0_MODE_BITMAP;
        _attr0 |= GFX_OAM_ATTR0_MODE_TRANSLUCENT;
        return *this;
    }

    constexpr OamBuilder& AsBitmap(int alpha = 16)
    {
        _attr0 &= ~GFX_OAM_ATTR0_PLTT256;
        _attr0 |= GFX_OAM_ATTR0_MODE_BITMAP;

        if (alpha == 0)
            return Hidden();

        _attr2 &= ~GFX_OAM_ATTR2_BMP_ALPHA(0xF);
        _attr2 |= GFX_OAM_ATTR2_BMP_ALPHA(alpha - 1);
        return *this;
    }

    constexpr OamBuilder& WithAffineMtx(int mtx, bool doubleSize = false)
    {
        _attr0 &= ~GFX_OAM_ATTR0_DOUBLE_AFFINE;
        if (doubleSize)
            _attr0 |= GFX_OAM_ATTR0_DOUBLE_AFFINE;
        else
            _attr0 |= GFX_OAM_ATTR0_AFFINE;
        _attr1 &= ~GFX_OAM_ATTR1_MTX(0x1F);
        _attr1 |= GFX_OAM_ATTR1_MTX(mtx);
        return *this;
    }

    constexpr OamBuilder& WithHFlip(bool hFlip = true)
    {
        if (hFlip)
            _attr1 |= GFX_OAM_ATTR1_HFLIP;
        else
            _attr1 &= ~GFX_OAM_ATTR1_HFLIP;
        return *this;
    }

    constexpr OamBuilder& WithVFlip(bool vFlip = true)
    {
        if (vFlip)
            _attr1 |= GFX_OAM_ATTR1_VFLIP;
        else
            _attr1 &= ~GFX_OAM_ATTR1_VFLIP;
        return *this;
    }

    constexpr OamBuilder& WithPriority(int priority)
    {
        _attr2 &= ~GFX_OAM_ATTR2_PRIORITY(3);
        _attr2 |= GFX_OAM_ATTR2_PRIORITY(priority);
        return *this;
    }

    constexpr OamBuilder& WithPalette16(int palette)
    {
        _attr0 &= ~GFX_OAM_ATTR0_PLTT256;
        _attr2 &= ~GFX_OAM_ATTR2_PLTT(0xF);
        _attr2 |= GFX_OAM_ATTR2_PLTT(palette);
        return *this;
    }

    constexpr OamBuilder& WithPalette256(int palette = 0)
    {
        _attr0 |= GFX_OAM_ATTR0_PLTT256;
        _attr2 &= ~GFX_OAM_ATTR2_PLTT(0xF);
        _attr2 |= GFX_OAM_ATTR2_PLTT(palette);
        return *this;
    }

    constexpr void Build(gfx_oam_entry_t& oamEntry) const
    {
        oamEntry.attr0 = _attr0;
        oamEntry.attr1 = _attr1;
        oamEntry.attr2 = _attr2;
    }
};