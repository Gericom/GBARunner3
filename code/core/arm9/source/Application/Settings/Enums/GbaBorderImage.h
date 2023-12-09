#pragma once

/// @brief Enum representing the type of border image to use.
enum class GbaBorderImage
{
    /// @brief No border image is used. The border will be black.
    None,

    /// @brief When available, the default border image will be used. Otherwise the border will be black.
    Default,

    /// @brief When available, a game specific border image will be used. Otherwise the default border will be used.s
    Game
};
