.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"

.macro memu_thumbLdrImmRd rd
    arm_func memu_thumbLdrImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #4

        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
            addlo r9, r9, r10, lsr #22
        ldr r10, [r9, #1] // memu_load32Table
        // interlock
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrImmRd, 8

.macro memu_thumbLdrhImmRd rd
    arm_func memu_thumbLdrhImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #5
        
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
            addlo r9, r9, r10, lsr #22
        ldr r10, [r9, #0x41] // memu_load16Table
        // interlock
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrhImmRd, 8

.macro memu_thumbLdrbImmRd rd
    arm_func memu_thumbLdrbImmR\rd
        and r8, r11, #0x7C0
        add r8, r10, r8, lsr #6
       
        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
            addlo r9, r9, r10, lsr #22
        ldr r10, [r9, #0x81] // memu_load8Table
        // interlock
        blx r10

        and r\rd, r9, #0xFF
        memu_thumbReturn
.endm

generate memu_thumbLdrbImmRd, 8

.end
