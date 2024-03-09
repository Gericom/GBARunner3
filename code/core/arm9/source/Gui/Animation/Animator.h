#pragma once
#include "Core/Math/fixed.h"
#include "Curve.h"

template <typename T>
class Animator
{
    T _from;
    T _to;
    T _value;
    u32 _duration;
    fix32<26> _invDuration;
    u32 _frame;
    const Curve* _curve;

public:
    constexpr Animator()
        : _from(), _to(), _value(), _duration(0), _frame(0), _curve(nullptr) { }

    /// @brief Constructs an Animator with the given initialValue.
    /// @param initialValue The initial value for the animator.
    constexpr Animator(const T& initialValue)
        : _from(initialValue), _to(initialValue), _value(initialValue), _duration(0), _frame(0), _curve(nullptr) { }

    /// @brief Constructs an Animator that animates from from to to with the given duration and curve.
    /// @param from The start value.
    /// @param to The end value.
    /// @param duration The duration of the animation.
    /// @param curve The curve to use for the animation.
    constexpr Animator(const T& from, const T& to, u32 duration, const Curve* curve)
        : _from(from), _to(to), _value(_from), _duration(duration), _invDuration(fix32<26>(1) / duration), _frame(0), _curve(curve) { }

    bool Update();

    constexpr const T& GetValue() const { return _value; }
    constexpr const T& GetTargetValue() const { return _to; }
    constexpr u32 GetDuration() const { return _duration; }
    constexpr u32 GetFrame() const { return _frame; }
    constexpr bool IsFinished() const { return _frame == _duration; }

    /// @brief Restarts this animator from the current value to a new target
    ///        with the given duration and curve.
    /// @param target The new target value.
    /// @param duration The duration of the animation.
    /// @param curve The curve to use.
    void Goto(const T& target, u32 duration, const Curve* curve)
    {
        *this = Animator(_value, target, duration, curve);
    }
};
