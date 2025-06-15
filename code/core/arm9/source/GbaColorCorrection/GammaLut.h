#pragma once

#include "common.h"
#include <algorithm>
#include <array>
#include <cmath>

class GammaLut
{
    static constexpr int TableSize = 256;
    static constexpr int GammaSteps = 5; // Min 1, Max 5. Default 5

    // [TARGET_GAMMA] Target Gamma range. According to the libretro docs, the DS gamma average range is 2.0f
    static constexpr float TargetGamma = 2.f; // Default 2.f
    static constexpr float DarkenScreen = 0.f; // Default 0.f

    // [DISPLAY_GAMMA] Display Gamma range can be modified from 0.f ~ 2.2f, 5 steps max
    static constexpr float GammaMin = 0.5f; // default 0.5f
    static constexpr float GammaMax = 0.9f; // default 0.9f
    static constexpr float GammaStep = (GammaMax - GammaMin) / (GammaSteps - 1);

    std::array<u8, TableSize> _encodeTable{};
    std::array<std::array<u8, TableSize>, GammaSteps> _decodeTables{};

public:
    constexpr GammaLut()
    {
        for (int i = 0; i < TableSize; ++i)
        {
            double x = static_cast<double>(i) / 255.0;
            _encodeTable[i] = static_cast<u8>(std::clamp(std::pow(x, TargetGamma + DarkenScreen) * 255.0, 0.0, 255.0));
        }

        for (int g = 0; g < GammaSteps; ++g)
        {
            double gamma = GammaMin + GammaStep * g;
            for (int i = 0; i < TableSize; ++i)
            {
                double x = static_cast<double>(i) / 255.0;
                _decodeTables[g][i] = static_cast<u8>(std::clamp(std::pow(x, gamma) * 255.0, 0.0, 255.0));
            }
        }
    }

    /// @brief Encodes a value using the [TARGET_GAMMA] curve.
    /// @param value Input value (0-255).
    /// @return Gamma encoded value (non-linear).
    constexpr u8 Encode(u8 value) const
    {
        return _encodeTable[value];
    }

    /// @brief Decodes a value using a precomputed [DISPLAY_GAMMA] curve.
    /// @param value Input value (0-255).
    /// @param gammaIndex Gamma steps (0 = 0.5f, 1 = 0.6f, 2 = 0.7f, 3 = 0.8f, 4 = 0.9f).
    /// @return Gamma decoded value (linearized).
    constexpr u8 Decode(u8 value, int gammaIndex) const
    {
        gammaIndex = std::clamp(gammaIndex, 0, GammaSteps - 1);
        return _decodeTables[gammaIndex][value];
    }
};

extern const GammaLut gGammaLut;