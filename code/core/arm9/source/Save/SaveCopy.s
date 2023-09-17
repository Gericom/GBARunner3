.text
.altmacro

#include "AsmMacros.inc"

/// @brief Copies byte by byte from a privileged source to a gba destination.
/// @param r0 Source address.
/// @param r1 Destination gba address.
/// @param r2 Length in bytes.
/// @param lr Return address.
arm_func sav_copyBytesToGba
    cmp r2, #0
        bxeq lr
1:
    ldrb r3, [r0], #1
    subs r2, r2, #1
    strbt r3, [r1], #1
    bne 1b
    bx lr

/// @brief Copies byte by byte from a gba source to a privileged destination.
/// @param r0 Source gba address.
/// @param r1 Destination address.
/// @param r2 Length in bytes.
/// @param lr Return address.
arm_func sav_copyBytesFromGba
    cmp r2, #0
        bxeq lr
1:
    ldrbt r3, [r0], #1
    subs r2, r2, #1
    strb r3, [r1], #1
    bne 1b
    bx lr

/// @brief Reads a single byte from the given gba address.
/// @param r0 The gba address to read a byte from.
/// @param lr Return address.
arm_func sav_readGbaByte
    ldrbt r0, [r0]
    bx lr

/// @brief Writes a single byte to the given gba address.
/// @param r0 The gba address to write a byte to.
/// @param r1 The byte to write.
/// @param lr Return address.
arm_func sav_writeGbaByte
    strbt r1, [r0]
    bx lr
