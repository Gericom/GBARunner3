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
    ldr r8, [r8, r9, lsl #2]
    // interlock
    bx r8

.extern jit_handleThumbUndefined

arm_func vm_undefinedThumb
    ldr sp,= (dtcmStackEnd - 0x40)
    ldr r10, DTCM(vm_undefinedSpsr)
    stmia sp, {r0-lr}^
    nop
    add r1, r11, #2
    str r1, [sp, #0x3C]
    sub r1, r11, #2

    bic r0, r1, #0x01000000
    add r0, r0, #0x00400000

    ldrh r0, [r0]
    mov r2, sp
    mov r3, r10
#ifndef GBAR3_TEST
    bl jit_handleThumbUndefined
#endif
    movs r8, r0, lsr #1
        biccc r10, r10, #0x20 // thumb bit
    ldmia sp, {r0-lr}^
    nop
    msr spsr, r10
    movs pc, r8, lsl #1

.section ".dtcm", "aw"

.macro vm_armUndefinedInstructionPointer2 prefix, d, e, f, g, h, x, y
    .if (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 0)
        // MSR
        .if \h == 1
            @ .short vm_armUndefinedMsrImm\f
            .short 0
        .else
            .if \f == 1
                .short vm_armUndefinedMsrRegSpsrRmTable
            .else
                .short vm_armUndefinedMsrRegCpsrRmTable
            .endif
        .endif
    .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 1) && (\h == 0)
        // MRS
        .if \f == 1
            .short vm_armUndefinedMrsSpsrRmTable
        .else
            .short vm_armUndefinedMrsCpsrRmTable
        .endif
#ifndef GBAR3_TEST
    .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\f == 0) && (\g == 1) && (\h == 1)
        // BX
        .short jit_armUndefinedBxRmTable
    .elseif (\prefix == 0b110) && (\d == 0)
        // B
        .short jit_armUndefinedBTable
    .elseif (\prefix == 0b110) && (\d == 1)
        // BL
        .short jit_armUndefinedBLTable
    .elseif (\prefix == 0b111) && (\d == 0) && (\e == 1) && (\f == 0) && (\g == 0) && (\h == 0) && (\x == 0) && (\y == 1)
        // LDR pc, [Rn, +/-#imm] or LDR pc, [Rn], +/-#imm
        .short jit_armUndefinedLdrPcImmRnTable
#endif
    .elseif (\prefix == 0b111) && (\d == 0) && (\e == 0) && (\y == 0)
        // ALU pc
        .if (\f == 1) && (\g == 1)
            // ALUs pc, rn, #imm
            .short vm_armUndefinedAluSPCImmRnTable
        .else
            .short 0
        .endif
    .else
        .short 0
    .endif
.endm

.macro vm_armUndefinedInstructionPointer index
    vm_armUndefinedInstructionPointer2 %((\index>>5)&7), %((\index>>4)&1), %((\index>>3)&1), %((\index>>2)&1), %((\index>>1)&1), %((\index>>0)&1), %((\index>>8)&1), %((\index>>9)&1)
.endm

generate vm_armUndefinedInstructionPointer, 1024
