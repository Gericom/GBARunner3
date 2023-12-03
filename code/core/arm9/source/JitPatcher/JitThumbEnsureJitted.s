.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/RomDefs.h"

arm_func jit_thumbEnsureJittedHiReg
    ldr r8, [sp, #-4]

arm_func jit_thumbEnsureJitted
    cmp r8, #ROM_LINEAR_GBA_ADDRESS
        addhs r8, r8, #(ROM_LINEAR_DS_ADDRESS - ROM_LINEAR_GBA_ADDRESS)
    sub lr, r8, #ROM_LINEAR_DS_ADDRESS
    cmp lr, #ROM_LINEAR_SIZE
    bhs 1f

    ldr r12,= gJitState // staticRomJitBits
    movs lr, lr, lsr #1
        biccc r10, r10, #0x20 // thumb bit
    ldrb r12, [r12, lr, lsr #3]
    and lr, lr, #0x7
    rsb lr, lr, #32
    movs r12, r12, lsl lr
    msrcs spsr, r10
    movcss pc, r8

1:
    tst r8, #1
        biceq r10, r10, #0x20 // thumb bit

    and lr, r8, #0x0FF00000
    cmp lr, #0x06800000
        beq 2f

    mov lr, r11, lsr #24
    cmp lr, #2
        moveq lr, #0
        mcreq p15, 0, lr, c7, c10, 4
        mcreq p15, 0, lr, c7, c5, 0

    push {r0-r3}
    mov r0, r8
    bl jit_ensureBlockJitted
    pop {r0-r3}
2:
    msr spsr, r10
    movs pc, r8
