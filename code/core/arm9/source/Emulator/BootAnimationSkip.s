.text
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

// based on the bootrom overlay used in open agb firm
arm_func emu_resetVectorSkipBootAnimation
    mov r0, #1
    mov r1, #0x4000000
    strb r0, [r1, #0x300] // POSTFLG

    // svc mode stack
    ldr r1,= 0x3007E1D
    mov r1, #0x93
    vmMSR cpsr, r1
    ldr sp,= 0x3007E88

    // system mode stack
    mov r1, #0x9F
    vmMSR cpsr, r1
    ldr sp,= 0x3007E78

    mov r4, #0x04000000
    mov r0, #0xFF
    swi 0x010000 // RegisterRamReset
    ldr r0,= gGbaBios + 0xBC // SoftReset (0xB4) but skipping r2 & r4 loading.
    mov r2, #0
1:
    ldrb r1, [r4, #6] // REG_VCOUNT
    cmp r1, #126 // Loop until REG_VCOUNT == 126.
    bne 1b
    bx r0
