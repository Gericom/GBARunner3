.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

/// @brief Handles 16-bit stores to GBA REG_DISPCNT and updates DS REG_DISPCNT accordingly.
/// @param r0-r7 Preserved
/// @param r8 Address; should be 0x04000000. The bottom 2 bits are ignored.
/// @param r9 Value; the top 16 bits are ignored. Trashed.
/// @param r10-r12 Trashed
/// @param r13 Preserved
/// @param lr Return address.
arm_func emu_regDispCntStore16
    mov r10, r13
    ldr sp,= dtcmStack + 128
    push {r0-r3,r10,lr}
    mov r0, r9, lsl #16
    mov r0, r0, lsr #16
    bl emu_regDispCntStore
    ldmfd sp, {r0-r3,r13,pc}
