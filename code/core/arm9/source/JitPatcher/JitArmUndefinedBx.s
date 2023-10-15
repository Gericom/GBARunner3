.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_armUndefinedBxRm rm
    arm_func jit_armUndefinedBxR\rm
        ldr r9, [r13, #(vm_undefinedRegTmp - vm_armUndefinedDispatchTable)]! // dummy read, only used to compute an address
        .if \rm < 8
            mov r8, r\rm
            b ensureJittedCommon
        .elseif \rm < 15
            stmia r13, {r\rm}^
            b ensureJittedCommonHiReg
        .else
            // pc
            add r8, r11, #4
            b ensureJittedCommon
        .endif
.endm

generate jit_armUndefinedBxRm, 16

ensureJittedCommonHiReg:
    ldr r8, [r13]

ensureJittedCommon:
    cmp r8, #0x08000000
        addhs r8, r8, #(0x02200000 - 0x08000000)

    ldr r10, [r13, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
    ldr sp,= dtcmStackEnd
    tst r8, #1
        orrne r10, r10, #0x20 // thumb bit
    sub r9, r8, #0x02200000
    cmp r9, #0x00200000
    blo ensureJittedStaticRom
    sub r9, r8, #0x03000000
    cmp r9, #0x8000
    blo ensureJittedIWram
1:
    push {r0-r3}
    mov r0, r8
    bl jit_ensureBlockJitted
    pop {r0-r3}
2:
    msr spsr, r10
    movs pc, r8

ensureJittedStaticRom:
    ldr r11,= gJitState // staticRomJitBits
    mov r9, r9, lsr #1
    ldrb r11, [r11, r9, lsr #3]
    and r9, r9, #0x7
    rsb r9, r9, #32
    movs r11, r11, lsl r9
    msrcs spsr, r10
    movcss pc, r8
    b 1b

ensureJittedIWram:
    mov lr, r11, lsr #24
    cmp lr, #2
        moveq lr, #0
        mcreq p15, 0, lr, c7, c10, 4
        mcreq p15, 0, lr, c7, c5, 0

    ldr r11,= (gJitState + 0x20000) // iWramJitBits
    mov r9, r9, lsr #1
    ldrb r11, [r11, r9, lsr #3]
    and r9, r9, #0x7
    rsb r9, r9, #32
    movs r11, r11, lsl r9
    msrcs spsr, r10
    movcss pc, r8
    b 1b

.section ".dtcm", "aw"

.global jit_armUndefinedBxRmTable
jit_armUndefinedBxRmTable:
    .word jit_armUndefinedBxR0
    .word jit_armUndefinedBxR1
    .word jit_armUndefinedBxR2
    .word jit_armUndefinedBxR3
    .word jit_armUndefinedBxR4
    .word jit_armUndefinedBxR5
    .word jit_armUndefinedBxR6
    .word jit_armUndefinedBxR7
    .word jit_armUndefinedBxR8
    .word jit_armUndefinedBxR9
    .word jit_armUndefinedBxR10
    .word jit_armUndefinedBxR11
    .word jit_armUndefinedBxR12
    .word jit_armUndefinedBxR13
    .word jit_armUndefinedBxR14
    .word jit_armUndefinedBxR15

.text

.macro jit_armPlaceBackBxRm rm
    arm_func jit_armPlaceBackBxR\rm
        ldr r8,= (0x012FFF10 | (\rm))
        b jit_armPlaceBackBxRmCommon
.endm

generate jit_armPlaceBackBxRm, 16

arm_func jit_armPlaceBackBxRmCommon
    and r9, lr, #0xF0000000
    orr r8, r8, r9
    str r8, [r11, #-4]!
    mov r8, #0
    mcr p15, 0, r8, c7, c10, 4
    mcr p15, 0, r8, c7, c5, 0
    ldr r10, [r13, #(vm_undefinedSpsr - vm_armUndefinedDispatchTable)]
    msr spsr, r10
    movs pc, r11
