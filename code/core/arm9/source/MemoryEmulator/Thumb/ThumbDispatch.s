.section ".itcm", "ax"
.altmacro
.arm

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "../MemoryEmuDtcm.inc"

thumb_dispatch_base:

#define DTCM(x) #(thumb_dispatch_base - 0xC00 + x)

arm_func memu_thumbDispatch
    str r0, DTCM(memu_thumb_r0)
    ldrh r0, [lr, #-8]

@ #ifndef GBAR3_TEST
@     ldr sp,= dtcmStackEnd
@     push {r0-r3,r12,lr}
@     .extern logOpcode
@     ldr r12,= logOpcode
@     blx r12
@     pop {r0-r3,r12,lr}
@ #endif

    msr cpsr_c, #0xD1 // switch to fiq mode

    orr r11, r0, r0, lsl #16
    ldr r0, DTCM(memu_thumb_r0)
    mvn r9, #0x3000 // mask for dtcm mirroring; 0xFFFFCFFF
    bic r12, r9, r11, lsr #7 // dtcm must mirror
    ldrh r12, [r12] // r12 = address of instruction handler

    and r8, r11, #0x1F8 // r8 = (Rm_Rn) << 3
    add pc, pc, r8, lsl #1
    .word 0

.macro memu_thumbGetRnRm rn, rm
    arm_func memu_thumbGetR\rn\()R\rm
        mov r10, r\rn
        add r8, r\rn, r\rm
        bx r12
        .word 0
.endm

.macro make_memu_thumbGetRnRm arg
    memu_thumbGetRnRm %((\arg>>0)&7),%((\arg>>3)&7)
.endm

generate make_memu_thumbGetRnRm, 64

.end
