#pragma once
#include "Enums/GbaSaveType.h"

class GameSettings
{
public:
    /// @brief Specifies the save type to use.
    GbaSaveType saveType = GbaSaveType::Auto;
};
