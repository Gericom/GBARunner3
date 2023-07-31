.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_undefined_base:

#define DTCM(x) (vm_undefined_base - 0x600 + (x))

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

arm_func vm_undefinedThumb
    ldrh r13, [lr, #-2]
    msr cpsr_c, #0xD7
    movs pc, lr

.section ".dtcm", "aw"

.global vm_armUndefinedDispatchTable
vm_armUndefinedDispatchTable:

.macro vm_armUndefinedInstructionPointer2 prefix, d, e, f, g, h, x, y
    .if (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 0)
        // MSR
        @ .if \h == 1
        @     .short vm_armUndefinedMsrImm\f
        @ .else
        @     .short vm_armUndefinedMsrReg\f
        @ .endif
        .short 0
    .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 1) && (\h == 0)
        // MRS
        .if \f == 1
            .short vm_armUndefinedMrsSpsrRmTable
        .else
            .short vm_armUndefinedMrsCpsrRmTable
        .endif
    @ .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\f == 0) && (\g == 1) && (\h == 1)
    @     // BX
    @     .short vm_armUndefinedBx
    .else
        .short 0
    .endif
.endm

.macro vm_armUndefinedInstructionPointer index
    vm_armUndefinedInstructionPointer2 %((\index>>5)&7), %((\index>>4)&1), %((\index>>3)&1), %((\index>>2)&1), %((\index>>1)&1), %((\index>>0)&1), %((\index>>8)&1), %((\index>>9)&1)
.endm

generate vm_armUndefinedInstructionPointer, 1024
