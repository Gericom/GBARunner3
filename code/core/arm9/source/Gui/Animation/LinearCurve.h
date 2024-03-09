#pragma once
#include "Curve.h"

class LinearCurve : public Curve
{
public:
    fix32<26> Compute(fix32<26> t) const override
    {
        return t;
    }
};

constexpr auto gLinearCurve = LinearCurve();
