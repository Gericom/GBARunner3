#include <nds.h>
#include "Animator.h"

template <>
bool Animator<int>::Update()
{
    if (++_frame >= _duration)
    {
        _frame = _duration;
        _value = _to;
        return true;
    }

    auto relativePos = _curve->Compute(_frame * _invDuration);
    _value = _from + relativePos.LongMul(_to - _from).Int();

    return false;
}