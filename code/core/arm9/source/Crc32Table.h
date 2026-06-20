#pragma once

class Crc32Table
{
    u32 _table[256];

public:
    constexpr Crc32Table()
        : _table()
    {
        for (u32 i = 0; i < 256; i++)
        {
            u32 c = i;
            for (int k = 0; k < 8; k++)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            _table[i] = c;
        }
    }

    constexpr u32 Table(u8 index) const { return _table[index]; }
};

extern const Crc32Table gCrc32Table;
