.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_swi_base:

#define DTCM(x) (vm_swi_base - 0xB40 + (x))

arm_func vm_swi
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
    ldr pc, DTCM(vm_swiVector)
    nop
    nop
    nop
old_mode_fiq:
    add r13, lr, #(vm_regs_sys - vm_regs_svc)
    stmdb r13, {r8,r9,r10,r11,r12,r13,lr}^
    nop
    ldmia r13, {r8,r9,r10,r11,r12}^
    nop
    ldmia lr, {r13,lr}^
    ldr pc, DTCM(vm_swiVector)
    nop
old_mode_irq:
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    ldr pc, DTCM(vm_swiVector)
    nop
    nop
    nop
    nop
old_mode_svc:
    ldmib lr, {lr}^
    ldr pc, DTCM(vm_swiVector)
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_4:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_5:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_6:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_abt:
    add r13, lr, #(vm_regs_abt - vm_regs_svc)
    stmia r13, {r13,lr}^
    nop
    ldmdb lr, {r13,lr}^
    ldr pc, DTCM(vm_swiVector)
    nop
    nop
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
    ldr pc, DTCM(vm_swiVector)
    nop
    nop
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
    ldr pc, DTCM(vm_swiVector)
