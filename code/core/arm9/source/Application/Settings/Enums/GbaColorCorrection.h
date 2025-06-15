#pragma once

/// @brief Enum representing the type of color correction to use.
enum class GbaColorCorrection
{
    /// @brief No color correction is applied.
    None,

    /// @brief Color correction is applied that resembles the AGB-001 GBA screen.
    Agb001,

    /// @brief Color correction is applied that resembles the AGS-101 GBA SP Bright screen.
    Ags101,

    /// @brief Color correction is applied that resembles the OXY-001 GB micro screen.
    Oxy001,

    /// @brief Color correction is applied that resembles the NTR-001 NDS Phat screen.
    Ntr001,

    /// @brief Color correction is applied that resembles the USG-001 NDS Lite screen.
    Usg001,

    /// @brief Color correction is applied that resembles the PSP-01g screen.
    Psp01g,

    /// @brief Color correction is applied that resembles the Nintendo Switch Classics GBA shader.
    NswIps,

    /// @brief Color correction is applied that resembles the Nintendo Switch Classics GBA shader in OLED screen.
    NswOle,

    /// @brief Color correction is applied that resembles the VisualBoy Advance shader.
    VbaEmu,

    /// @brief Color correction is applied that resembles the No$GBA shader.
    NoCash,

    /// @brief Color correction is applied that resembles the mGBA default shader.
    mGba01
};
