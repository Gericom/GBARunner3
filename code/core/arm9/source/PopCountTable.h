#pragma once

class PopCountTable
{
    u8 _table[256];

public:
    constexpr PopCountTable()
        : _table()
    {
        for (auto i = 0; i < 256; i++)
            _table[i] = __builtin_popcount(i);
    }

    constexpr u32 PopCount(u8 x) const { return _table[x]; }
    constexpr u32 PopCount(u16 x) const { return _table[x & 0xFF] + _table[x >> 8]; }
    constexpr u32 PopCount(u32 x) const
    {
        return _table[x & 0xFF] + _table[(x >> 8) & 0xFF] +
            _table[(x >> 16) & 0xFF] + _table[x >> 24];
    }
};

extern const PopCountTable gPopCountTable;
