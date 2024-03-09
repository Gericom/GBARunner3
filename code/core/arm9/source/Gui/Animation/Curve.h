#pragma once
#include "Core/Math/fixed.h"

class Curve
{
public:
    virtual fix32<26> Compute(fix32<26> t) const = 0;
};