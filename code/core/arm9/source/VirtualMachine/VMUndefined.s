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
    ldr r13, DTCM(vm_undefinedArmTableAddr)
    and r8, lr, #0x0FF00000
    and r9, lr, #0x810
    orr r9, r9, r9, lsr #8
    orr r8, r8, r9, lsl #25
    mov r8, r8, lsr #19
    ldrsh r8, [r13, r8]
    and r9, lr, #0xF
    // interlock
    cmp r8, #0
    ldrne r8, [r8, r9, lsl #2]
    beq vm_armUndefinedNoAsmHandler
    bx r8

.extern jit_handleThumbUndefined

arm_func vm_undefinedThumb
    ldrh lr, [r11, #-2]!
    ldr sp,= (dtcmStackEnd - 0x40)
    ldr r12, DTCM(vm_undefinedThumbTableAddr)
    mov r10, lr, lsl #19
    mov r10, r10, lsr #22
    ldrh r12, [r12, r10]
    ldr r10, DTCM(vm_undefinedSpsr)
    bx r12

arm_func vm_undefinedThumbNotInTable
    add r12, r11, #4
    str r12, [sp, #0x3C]
    stmia sp, {r0-lr}^
    nop
    mov r1, r11 // instruction address
    mov r0, lr  // instruction
    mov r2, sp  // registers
    mov r3, r10 // spsr
#ifndef GBAR3_TEST
    bl jit_handleThumbUndefined
#endif
    movs r8, r0, lsr #1
        biccc r10, r10, #0x20 // thumb bit
    ldmia sp, {r0-lr}^
    nop
    msr spsr, r10
    movs pc, r8, lsl #1

.end
