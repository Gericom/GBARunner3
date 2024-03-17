.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_gbaSoundStore8
    mov r11, #1
gbaSoundStore:
    push {r0-r3,lr}
    mov r0, r8
    mov r1, r9
    mov r2, r11
    bl gbas_writeSoundRegister
    pop {r0-r3,pc}

arm_func emu_gbaSoundStore16
    mov r11, #2
    b gbaSoundStore

arm_func emu_gbaSoundStore32
    mov r11, #4
    b gbaSoundStore

arm_func emu_gbaSoundLoadSoundCntX
    ldr r10,= gGbaSoundShared
    mov r11, #0
    mcr p15, 0, r11, c7, c10, 4 // drain write buffer
    mcr p15, 0, r10, c7, c6, 1 // invalidate range
    ldrh r9, [r10]
    tst r9, #0xFF00
        moveq r9, #0 // when master enable off
        andne r9, r9, #0xF // when master enable on
        orrne r9, r9, #0x80
    bx lr
