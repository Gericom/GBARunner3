#pragma once

class Environment
{
    enum EnvironmentFlags : u32
    {
        ENVIRONMENT_FLAGS_NONE = 0,

        ENVIRONMENT_FLAGS_DSI_MODE = (1 << 0),
        ENVIRONMENT_FLAGS_NOCASH_PRINT = (1 << 1),
        ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR = (1 << 2),
        ENVIRONMENT_FLAGS_JTAG_SEMIHOSTING = (1 << 3),
        ENVIRONMENT_FLAGS_AGB_SEMIHOSTING = (1 << 4),
        ENVIRONMENT_FLAGS_DLDI = (1 << 5),
        ENVIRONMENT_FLAGS_ARGV = (1 << 6),
        ENVIRONMENT_FLAGS_PICO_AGB_ADAPTER = (1 << 7)
    };

    static u32 _flags;

public:
    static void Initialize();

    static inline bool IsDsiMode() { return _flags & ENVIRONMENT_FLAGS_DSI_MODE; }
    static inline bool SupportsNocashPrint() { return _flags & ENVIRONMENT_FLAGS_NOCASH_PRINT; }
    static inline bool IsIsNitroEmulator() { return _flags & ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR; }
    static inline bool SupportsJtagSemihosting() { return _flags & ENVIRONMENT_FLAGS_JTAG_SEMIHOSTING; }
    static inline bool SupportsAgbSemihosting() { return _flags & ENVIRONMENT_FLAGS_AGB_SEMIHOSTING; }
    static inline bool SupportsDldi() { return _flags & ENVIRONMENT_FLAGS_DLDI; }
    static inline bool SupportsArgv() { return _flags & ENVIRONMENT_FLAGS_ARGV; }
    static inline bool HasPicoAgbAdapter() { return _flags & ENVIRONMENT_FLAGS_PICO_AGB_ADAPTER; }
};