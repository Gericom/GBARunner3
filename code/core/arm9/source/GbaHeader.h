#pragma once

struct GbaHeader
{
    u32 romEntryPoint;
    u8 logoData[156];
    char gameTitle[12];
    u32 gameCode;
    u16 makerCode;
    u8 fixedValue;
    u8 machineCode;
    u8 deviceType;
    u8 reservedB5[7];
    u8 softwareVersion;
    u8 headerChecksum;
    u16 reservedBE;
};

static inline consteval u32 GAMECODE(const char code[4])
{
    return code[0] | (code[1] << 8) | (code[2] << 16) | (code[3] << 24);
}

