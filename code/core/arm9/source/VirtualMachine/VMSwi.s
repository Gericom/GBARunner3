.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_swi_base:

#define DTCM(x) (vm_swi_base - 0x7D8 + (x))

.extern sav_swiHandler

arm_func vm_swi
    // check if this is a save patch swi
#ifndef GBAR3_TEST
#ifdef GBAR3_HICODE_CACHE_MAPPING
    cmp lr, #0x08000000
        bhs hicodeLoadFromCache
#endif
    ldrb r13, [lr, #-2]
hicodeContinue:
    cmp r13, #0x80
        bhs sav_swiHandler
#endif

    mov r13, #0
#ifdef GBAR3_HICODE_CACHE_MAPPING
    mcr p15, 0, r13, c9, c0, 1 // unlock icache
    mcr	p15, 0, r13, c6, c4, 0 // disable mpu region
#endif
    mcr p15, 0, r13, c7, c5, 0

    str lr, DTCM(vm_regs_svc + 4)
    ldr lr, DTCM(vm_cpsr)
    mrs r13, spsr
    bic r13, r13, #0xCF
    orr r13, r13, lr
    str r13, DTCM(vm_spsr_svc)
    tst r13, #0x40
    and r13, r13, #0xF
    mov lr, #0x93
    orrne lr, lr, #0x40
    str lr, DTCM(vm_cpsr)

    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    ldr lr, DTCM(vm_svcRegsAddr)
    add pc, pc, r13, lsl #5
    nop
old_mode_usr:
    add r13, lr, #((vm_regs_sys + 5 * 4) - vm_regs_svc)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
1:
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
    nop
old_mode_fiq:
    add r13, lr, #(vm_regs_sys - vm_regs_svc)
    stmdb r13, {r8,r9,r10,r11,r12,r13,lr}^
    nop
    ldmia r13, {r8,r9,r10,r11,r12}^
    nop
    ldmia lr, {r13,lr}^
    b 1b
    nop
old_mode_irq:
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
    nop
    nop
old_mode_svc:
    ldmib lr, {lr}^
    nop
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
    nop
    nop
    nop
    nop
old_mode_4:

#ifdef GBAR3_HICODE_CACHE_MAPPING
hicodeLoadFromCache:
    sub r13, lr, #2
    bic r13, r13, #0xFE000000
    tst r13, #0x800
    orrne r13, r13, #0x40000000 // set
    mcr p15, 3, r13, c15, c0, 0 // set index
    mrc p15, 3, r13, c15, c3, 0 // read data
    and r13, r13, #0xFF
    b hicodeContinue
#endif

.space 96 - (. - old_mode_4)

old_mode_abt:
    add r13, lr, #(vm_regs_abt - vm_regs_svc)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
    nop
old_mode_8:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_9:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_10:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_und:
    add r13, lr, #(vm_regs_und - vm_regs_svc)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
    nop
old_mode_12:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_13:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_14:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_sys:
    add r13, lr, #((vm_regs_sys + 5 * 4) - vm_regs_svc)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_swiVector)
    movs pc, lr
