.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "GbaIoRegOffsets.h"

arm_func emu_gbaSoundStore8
    mov r10, r13
    ldr sp,= dtcmSndStackEnd
    push {r0-r3,r10,lr}
    mov r0, r8
    mov r1, r9
    mov r2, #1
    bl gbas_writeSoundRegister
    ldmfd sp, {r0-r3,r13,pc}

arm_func emu_gbaSoundStore16
    mov r10, r13
    ldr sp,= dtcmSndStackEnd
    push {r0-r3,r10,lr}
    mov r0, r8
    mov r1, r9
    mov r2, #2
    bl gbas_writeSoundRegister
    ldmfd sp, {r0-r3,r13,pc}

arm_func emu_gbaSoundStore32
    mov r10, r13
    ldr sp,= dtcmSndStackEnd
    push {r0-r3,r10,lr}
    mov r0, r8
    mov r1, r9
    mov r2, #4
    bl gbas_writeSoundRegister
    ldmfd sp, {r0-r3,r13,pc}
