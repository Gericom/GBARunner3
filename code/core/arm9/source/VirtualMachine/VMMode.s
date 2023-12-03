.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_mode_base:

#define DTCM(x) (vm_mode_base - 0x474 + (x))

vm_regs_1 = vm_regs_fiq
vm_regs_2 = vm_regs_irq
vm_regs_3 = vm_regs_svc
vm_regs_7 = vm_regs_abt
vm_regs_11 = vm_regs_und
vm_regs_15 = vm_regs_sys

arm_func vm_modeSwitchOldEqualsNew
    bx r13

// note: ldm sp, {sp,lr}^ is a broken instruction according to arm946e-s errata

.macro vm_modeSwitchOldToNew old, new
    .if \old == \new
        .exitm
    .endif
    .if (\old != 1) && (\old != 2) && (\old != 3) && (\old != 7) && (\old != 0xB) && (\old != 0xF)
        .exitm
    .endif
    .if (\new != 1) && (\new != 2) && (\new != 3) && (\new != 7) && (\new != 0xB) && (\new != 0xF)
        .exitm
    .endif

    arm_func vm_modeSwitch\old\()To\new
        .if \new == 1
            .if \old == 0xF
                mov lr, #(vm_regs_sys + 3)
                stmia lr, {r8,r9,r10,r11,r12,sp,lr}^
                nop
                ldmdb lr, {r8,r9,r10,r11,r12,sp,lr}^
                nop
            .else
                mov lr, #(vm_regs_\old + 3)
                stmia lr, {sp,lr}^
                nop
                add lr, lr, #(vm_regs_sys - vm_regs_\old)
                stmia lr, {r8,r9,r10,r11,r12}^
                nop
                ldmdb lr, {r8,r9,r10,r11,r12,sp,lr}^
                nop
            .endif
        .elseif \old == 1
            mov lr, #(vm_regs_sys + 3)
            stmdb lr, {r8,r9,r10,r11,r12,sp,lr}^
            nop
            .if \new == 0xF
                ldmia lr, {r8,r9,r10,r11,r12,sp,lr}^
                nop
            .else
                ldmia lr, {r8,r9,r10,r11,r12}^
                nop
                add lr, lr, #(vm_regs_\new - vm_regs_sys)
                ldmia lr, {sp,lr}^
                nop
            .endif
        .elseif ((\old == 0xF) && (\new == 2)) || ((\old == 2) && (\new == 3)) || ((\old == 3) && (\new == 7)) || ((\old == 7) && (\new == 11))
            mov lr, #(vm_regs_\new + 3)
            stmdb lr, {sp,lr}^
            nop
            ldmia lr, {sp,lr}^
            nop
        .elseif ((\new == 0xF) && (\old == 2)) || ((\new == 2) && (\old == 3)) || ((\new == 3) && (\old == 7)) || ((\new == 7) && (\old == 11))
            mov lr, #(vm_regs_\old + 3)
            stmia lr, {sp,lr}^
            nop
            ldmdb lr, {sp,lr}^
            nop
        .else
            .if (\old == 0xF)
                mov lr, #(vm_regs_sys + (5 * 4) + 3)
            .else
                mov lr, #(vm_regs_\old + 3)
            .endif
            stmia lr, {sp,lr}^
            nop
            .if (\old == 0xF)
                add lr, lr, #(vm_regs_\new - (vm_regs_sys + (5 * 4)))
            .elseif (\new == 0xF)
                add lr, lr, #((vm_regs_sys + (5 * 4)) - vm_regs_\old)
            .else
                add lr, lr, #(vm_regs_\new - vm_regs_\old)
            .endif
            ldmia lr, {sp,lr}^
            nop
        .endif
        bx sp
.endm

.macro vm_modeSwitchOldToNew_gen index
    vm_modeSwitchOldToNew %((\index>>4)&0xF), %((\index>>0)&0xF)
.endm

generate vm_modeSwitchOldToNew_gen, 256

.section ".dtcm", "aw"

.macro vm_modeSwitchTableEntry2 old, new
    .if (\old != 0) && (\old != 1) && (\old != 2) && (\old != 3) && (\old != 7) && (\old != 0xB) && (\old != 0xF)
        .short 0
    .elseif (\new != 0) && (\new != 1) && (\new != 2) && (\new != 3) && (\new != 7) && (\new != 0xB) && (\new != 0xF)
        .short 0
    .elseif (\old == \new) || ((\old == 0) && (\new == 15)) || ((\old == 15) && (\new == 0))
        .short vm_modeSwitchOldEqualsNew
    .elseif \old == 0
        .short vm_modeSwitch15To\new
    .elseif \new == 0
        .short vm_modeSwitch\old\()To15
    .else
        .short vm_modeSwitch\old\()To\new
    .endif
.endm

.macro vm_modeSwitchTableEntry index
    vm_modeSwitchTableEntry2 %((\index>>0)&0xF), %((\index>>4)&0xF)
.endm

.global vm_modeSwitchTable
vm_modeSwitchTable:

generate vm_modeSwitchTableEntry, 256

.end
