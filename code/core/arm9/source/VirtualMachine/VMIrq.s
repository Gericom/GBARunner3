.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_irq_base:

#define DTCM(x) (vm_irq_base - 0xC0 + (x))

/// @brief Irq handler to be used when only emulation irqs should be handled,
///        and no irqs should be passed through to the VM. Should be branched
///        to directly from the hardware irq vector.
/// @param lr The return address + 4.
arm_func vm_nestedIrqHandler
    str r0, DTCM(vm_irqSavedR0)
    // todo: Don't tigger hblank irq on scanlines beyond the gba screen

    mov r13, #0x04000000
    ldr r0, [r13, #0x214]
    str lr, DTCM(vm_irqSavedLR)
    str r0, [r13, #0x214]
    ldr r13, DTCM(vm_hwIrqMask)
    ldr lr, DTCM(vm_emulatedIfImeIe)
    and r13, r13, r0 // bits to add to the emulated IF
    orr r13, lr, r13
    ldr lr, DTCM(vm_forcedIrqMask)
    str r13, DTCM(vm_emulatedIfImeIe)
    
    ands r0, r0, lr
    beq 1f

    tst r0, #(1 << 16) // ARM7 IRQ
        blne emu_arm7Irq
    tst r0, #2 // HBLANK IRQ
        blne emu_hblankIrq
    tst r0, #1 // VBLANK IRQ
        blne emu_vblankIrq

1:
    ldr lr, DTCM(vm_irqSavedLR)
    ldr r0, DTCM(vm_irqSavedR0)
    subs pc, lr, #4

/// @brief Main irq handler that handles both emulation irqs and passed
///        through irqs to the VM. Should be branched to directly from
///        the hardware irq vector.
/// @param lr The return address + 4.
arm_func vm_irq
    str r0, DTCM(vm_irqSavedR0)
    // here emulator irq tasks can be performed

    // todo: Don't tigger hblank irq on scanlines beyond the gba screen

    mov r13, #0x04000000
    ldr r0, [r13, #0x214]
    str lr, DTCM(vm_irqSavedLR)
    str r0, [r13, #0x214]
    ldr r13, DTCM(vm_hwIrqMask)
    ldr lr, DTCM(vm_emulatedIfImeIe)
    and r13, r13, r0 // bits to add to the emulated IF
    orr r13, lr, r13
    ldr lr, DTCM(vm_forcedIrqMask)
    str r13, DTCM(vm_emulatedIfImeIe)
    
    ands r0, r0, lr
    bne vm_emuIrq

vm_emu_irq_continue:
    ldr r13, DTCM(vm_emulatedIfImeIe)
    ldr r0, DTCM(vm_irqSavedR0)
    // 0EEE EEEE EEEE EEE0 M0FF FFFF FFFF FFFF (E = IE, M = IME, F = IF)
    //M0FFF FFFF FFFF FFF0
    tst r13, r13, lsl #17 // IF & IE
    ldr r13, DTCM(vm_irqSavedLR)
    ldr lr, DTCM(vm_cpsr)
    sublss pc, r13, #4 // IME = 0 or (IF & IE == 0)

    teq lr, lr, lsl #25 // C = I flag, N = F flag
    subcss pc, r13, #4 // vm doesn't want irqs; nothing changed; resume where we left off

    str r13, DTCM(vm_regs_irq + 4)

    mrs r13, spsr
    bic r13, r13, #0xCF
    orr r13, r13, lr
    str r13, DTCM(vm_spsr_irq)
    and r13, r13, #0xF
    mov lr, #0x92
    orrmi lr, lr, #0x40
    str lr, DTCM(vm_cpsr)

    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    mov lr, #(vm_regs_irq + 3)
    add pc, pc, r13, lsl #5
    nop
old_mode_usr:
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
1:
    ldr lr, DTCM(vm_irqVector)
    movs pc, lr
    nop
    nop
old_mode_fiq:
    add r13, lr, #(vm_regs_sys - vm_regs_irq)
    stmdb r13, {r8,r9,r10,r11,r12,r13,lr}^
    nop
    ldmia r13, {r8,r9,r10,r11,r12}^
    nop
    ldmia lr, {r13,lr}^
    b 1b
    nop
old_mode_irq:
    ldmib lr, {lr}^
    nop
    ldr lr, DTCM(vm_irqVector)
    movs pc, lr
    nop
    nop
    nop
    nop
old_mode_svc:
    add lr, lr, #(vm_regs_svc - vm_regs_irq)
    stmia lr, {r13,lr}^
    nop
    ldmdb lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_irqVector)
    movs pc, lr
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
    add r13, lr, #(vm_regs_abt - vm_regs_irq)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_irqVector)
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
    add r13, lr, #(vm_regs_und - vm_regs_irq)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_irqVector)
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
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    nop
    ldr lr, DTCM(vm_irqVector)
    movs pc, lr

vm_emuIrq:
    tst r0, #(1 << 16) // ARM7 IRQ
        blne emu_arm7Irq
    tst r0, #2 // HBLANK IRQ
        blne emu_hblankIrq
    tst r0, #1 // VBLANK IRQ
        blne emu_vblankIrq
    b vm_emu_irq_continue
