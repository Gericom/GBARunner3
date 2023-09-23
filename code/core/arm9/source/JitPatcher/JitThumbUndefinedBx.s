.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

.macro jit_thumbUndefinedBxRm rm
    arm_func jit_thumbUndefinedBxR\rm
        .if \rm < 8
            mov r8, r\rm
            b ensureJittedCommon
        .elseif \rm < 15
            stmdb sp, {r\rm}^
            b ensureJittedCommonHiReg
        .else
            // pc
            add r8, r11, #4
            bic r8, r8, #2 // 32 bit align target address
            b ensureJittedCommon
        .endif
.endm

generate jit_thumbUndefinedBxRm, 16

ensureJittedCommonHiReg:
    ldr r8, [sp, #-4]

ensureJittedCommon:
    sub lr, r8, #0x02200000
    cmp lr, #0x00200000
    bhs 1f

    ldr r12,= gStaticRomJitBits
    movs lr, lr, lsr #1
        biccc r10, r10, #0x20 // thumb bit
    ldrb r12, [r12, lr, lsr #3]
    and lr, lr, #0x7
    rsb lr, lr, #32
    movs r12, r12, lsl lr
    msrcs spsr, r10
    movcss pc, r8

1:
    mov lr, r11, lsr #24
    cmp lr, #2
    moveq lr, #0
    mcreq p15, 0, lr, c7, c10, 4
    mcreq p15, 0, lr, c7, c5, 0

    tst r8, #1
        biceq r10, r10, #0x20 // thumb bit
    push {r0-r3}
    mov r0, r8
    bl jit_ensureBlockJitted
    pop {r0-r3}
    msr spsr, r10
    movs pc, r8

@ arm_func jit_thumbBx
@     and r12, r0, #(0xF << 3)
@     ldr r12, [sp, r12, lsr #1]
@     tst r12, #1
@         biceq r12, r12, #2
@     sub lr, r12, #0x02200000
@     cmp lr, #0x00200000
@     bhs vm_undefinedThumbContinue

@ ensureJittedStaticRom:
@     ldr r11,= gStaticRomJitBits
@     movs lr, lr, lsr #1
@         biccc r10, r10, #0x20 // thumb bit
@     ldrb r11, [r11, lr, lsr #3]
@     and lr, lr, #0x7
@     rsb lr, lr, #32
@     movs r11, r11, lsl lr
@     bcc vm_undefinedThumbContinue
@     ldmia sp, {r0-r1}
@     msr spsr, r10
@     movs pc, r12
