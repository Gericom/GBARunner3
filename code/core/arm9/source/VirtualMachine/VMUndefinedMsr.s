.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

.macro vm_armUndefinedMsrRegCpsrRm rm
    arm_func vm_armUndefinedMsrRegCpsrR\rm
        ldr r9, [r13, #(vm_undefinedRegTmp - vm_armUndefinedDispatchTable)]! // dummy read, only used to compute an address
        .if \rm < 8
            mov r8, r\rm
        .elseif \rm < 15
            stmia r13, {r\rm}^
            nop
            ldr r8, [r13]
        .else
            // pc is not allowed
        .endif
        b vm_updateCpsr
.endm

vm_updateCpsr:
    mov r9, r13
    tst lr, #0x10000
    beq vm_cpsrNoControlChange // skip updating control part of cpsr
    ldr r10, [r9, #(vm_cpsr - vm_undefinedRegTmp)]
    and r13, r10, #0xF
    and r8, r8, #0xDF // note that the thumb bit is set to 0
    orr r8, r8, #0x10
    ldr r12, [r9, #(vm_modeSwitchTableAddr - vm_undefinedRegTmp)]
    strb r8, [r9, #(vm_cpsr - vm_undefinedRegTmp)]
    and r11, r8, #0xF
    add r12, r12, r11, lsl #5
    add r12, r12, r13, lsl #1
    ldrh r12, [r12]
    tst lr, #0x80000
    adreq r13, vm_finishCpsrOnlyControl
    adrne r13, vm_finishCpsrControlAndFlags
    bx r12

vm_cpsrNoControlChange:
    tst lr, #0x80000
    bne vm_finishCpsrWithFlags
vm_cpsrFinish:
    msr cpsr_c, #0xDB
    movs pc, lr

vm_finishCpsrWithFlags:
    ldrb r10, [r9, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
    and r8, r8, #0xF0000000
    ldr lr, [r9, #(vm_undefinedInstructionAddr - vm_undefinedRegTmp)]
    orr r10, r10, r8
    msr spsr, r10
    movs pc, lr
    
vm_finishCpsrOnlyControl:
    bic r11, r10, r8
    tst r11, #0x80
    beq vm_cpsrFinish
    // irqs were enabled
    ldr r11, [r9, #(vm_emulatedIfImeIe - vm_undefinedRegTmp)]
    tst r11, r11, lsl #17 // IF & IE
    bls vm_cpsrFinish // no irqs to handle
    ldr r10, [r9, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
    ldr lr, [r9, #(vm_undefinedInstructionAddr - vm_undefinedRegTmp)]
    msr spsr, r10
    add lr, lr, #4 // because irq return is subs pc, lr, #4
    b vm_irq

vm_finishCpsrControlAndFlags:
    bic r11, r10, r8
    tst r11, #0x80
    beq vm_finishCpsrWithFlags
    // irqs were enabled
    ldr r11, [r9, #(vm_emulatedIfImeIe - vm_undefinedRegTmp)]
    tst r11, r11, lsl #17 // IF & IE
    bls vm_finishCpsrWithFlags // no irqs to handle
    ldrb r10, [r9, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
    and r8, r8, #0xF0000000
    ldr lr, [r9, #(vm_undefinedInstructionAddr - vm_undefinedRegTmp)]
    orr r10, r10, r8
    msr spsr, r10
    add lr, lr, #4 // because irq return is subs pc, lr, #4
    b vm_irq

generate vm_armUndefinedMsrRegCpsrRm, 16

.macro vm_armUndefinedMsrRegSpsrRm rm
    arm_func vm_armUndefinedMsrRegSpsrR\rm
        ldr r9, [r13, #(vm_cpsr - vm_armUndefinedDispatchTable)]!
        .if \rm < 8
            mov r8, r\rm
        .elseif \rm < 15
            add r12, r13, #(vm_undefinedRegTmp - vm_cpsr)
            stmia r12, {r\rm}^
            nop
            ldr r8, [r12]
        .else
            // pc is not allowed
        .endif
        b vm_updateSpsr
.endm

generate vm_armUndefinedMsrRegSpsrRm, 16

vm_updateSpsr:
    and r9, r9, #0xF
    add r12, r13, r9, lsl #2
    orr r8, r8, #0x10
    ldr r10, [r12, #(vm_spsr - vm_cpsr)]
    mov r11, #0
    tst lr, #0x80000
    movne r11, #0xF0000000
    tst lr, #0x10000
    orrne r11, r11, #0xFF
    and r8, r8, r11
    bic r10, r10, r11
    orr r10, r10, r8
    str r10, [r12, #(vm_spsr - vm_cpsr)]
    msr cpsr_c, #0xDB
    movs pc, lr

.section ".dtcm", "aw"

.global vm_armUndefinedMsrRegCpsrRmTable
vm_armUndefinedMsrRegCpsrRmTable:
    .word vm_armUndefinedMsrRegCpsrR0
    .word vm_armUndefinedMsrRegCpsrR1
    .word vm_armUndefinedMsrRegCpsrR2
    .word vm_armUndefinedMsrRegCpsrR3
    .word vm_armUndefinedMsrRegCpsrR4
    .word vm_armUndefinedMsrRegCpsrR5
    .word vm_armUndefinedMsrRegCpsrR6
    .word vm_armUndefinedMsrRegCpsrR7
    .word vm_armUndefinedMsrRegCpsrR8
    .word vm_armUndefinedMsrRegCpsrR9
    .word vm_armUndefinedMsrRegCpsrR10
    .word vm_armUndefinedMsrRegCpsrR11
    .word vm_armUndefinedMsrRegCpsrR12
    .word vm_armUndefinedMsrRegCpsrR13
    .word vm_armUndefinedMsrRegCpsrR14
    .word vm_armUndefinedMsrRegCpsrR15

.global vm_armUndefinedMsrRegSpsrRmTable
vm_armUndefinedMsrRegSpsrRmTable:
    .word vm_armUndefinedMsrRegSpsrR0
    .word vm_armUndefinedMsrRegSpsrR1
    .word vm_armUndefinedMsrRegSpsrR2
    .word vm_armUndefinedMsrRegSpsrR3
    .word vm_armUndefinedMsrRegSpsrR4
    .word vm_armUndefinedMsrRegSpsrR5
    .word vm_armUndefinedMsrRegSpsrR6
    .word vm_armUndefinedMsrRegSpsrR7
    .word vm_armUndefinedMsrRegSpsrR8
    .word vm_armUndefinedMsrRegSpsrR9
    .word vm_armUndefinedMsrRegSpsrR10
    .word vm_armUndefinedMsrRegSpsrR11
    .word vm_armUndefinedMsrRegSpsrR12
    .word vm_armUndefinedMsrRegSpsrR13
    .word vm_armUndefinedMsrRegSpsrR14
    .word vm_armUndefinedMsrRegSpsrR15
