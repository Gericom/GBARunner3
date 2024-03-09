#pragma once

struct Point
{
    int x;
    int y;

    constexpr Point()
        : x(0), y(0) { }

    constexpr Point(int x, int y)
        : x(x), y(y) { }

    constexpr s64 DistanceSquaredTo(const Point& other) const
    {
        return (s64)(other.x - x) * (other.x - x) + (s64)(other.y - y) * (other.y - y);
    }
};
