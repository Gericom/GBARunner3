.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_undefined_base:

#define DTCM(x) (vm_undefined_base - 0x20 + (x))

arm_func vm_undefined
    str lr, DTCM(vm_undefinedInstructionAddr)
    mrs r13, spsr
    str r13, DTCM(vm_undefinedSpsr)
    tst r13, #0x20
    msr cpsr_c, #0xD1 // switch to fiq mode
    ldr r11, DTCM(vm_undefinedInstructionAddr)
    bne vm_undefinedThumb
    ldr lr, [r11, #-4] // lr = instruction
    ldr r12, DTCM(vm_undefinedArmTableAddr)
    and r8, lr, #0x0FF00000
    and r9, lr, #0x810
    orr r9, r9, r9, lsr #8
    orr r8, r8, r9, lsl #25
    ldrb r8, [r12, r8, lsr #20]
    and r9, lr, #0xF
    mov r9, r9, lsl #2
    cmp r8, #0
    ldrne r8, [r9, -r8, lsl #6]
    beq vm_armUndefinedNoAsmHandler
    bx r8

.extern jit_handleThumbUndefined

arm_func vm_undefinedThumb
    ldrh lr, [r11, #-2]!
    ldr r10, DTCM(vm_undefinedSpsr)
    ldr r12, DTCM(vm_undefinedThumbTableAddr)
    mov r8, lr, lsl #19
    mov r8, r8, lsr #22
    ldrh r12, [r12, r8]
    bx r12

arm_func vm_undefinedThumbNotInTable
    sub sp, sp, #0x28
    stmia sp, {r0-r7,sp,lr}^
    nop
    mov r1, r11 // instruction address
    mov r0, lr  // instruction
    mov r2, sp  // registers
#ifndef GBAR3_TEST
    bl jit_handleThumbUndefined
#endif
    mov r8, r0
    add sp, sp, #0x28
    ldmdb sp, {r0-r7,sp,lr}^
#ifndef GBAR3_TEST
    b jit_thumbEnsureJitted
#endif

.end
