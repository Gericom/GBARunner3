.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"
#include "VMUndefinedInstructions.inc"

vm_irq_base:

#define DTCM(x) (vm_irq_base - 0xB8 + (x))

/// @brief Irq handler to be used when only emulation irqs should be handled,
///        and no irqs should be passed through to the VM. Should be branched
///        to directly from the hardware irq vector.
/// @param lr The return address + 4.
arm_func vm_nestedIrqHandler
    str r0, DTCM(vm_irqSavedR0)

    mov r13, #0x04000000
    ldr r0, [r13, #0x214]
    str lr, DTCM(vm_irqSavedLR)
    str r0, [r13, #0x214]

    // Don't tigger hblank irq on scanlines beyond the gba screen
    ldrh r13, [r13, #6]
    cmp r13, #260
    bichs r0, r0, #2 // HBLANK IRQ
    sub r13, r13, #160
    subs r13, r13, #32
    biclo r0, r0, #2 // HBLANK IRQ

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

    mov r13, #0x04000000
    ldr r0, [r13, #0x214]
    str lr, DTCM(vm_irqSavedLR)
    str r0, [r13, #0x214]

    // Don't tigger hblank irq on scanlines beyond the gba screen
    ldrh r13, [r13, #6]
    cmp r13, #260
    bichs r0, r0, #2 // HBLANK IRQ
    sub r13, r13, #160
    subs r13, r13, #32
    biclo r0, r0, #2 // HBLANK IRQ

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

    // only forced irqs stay on in DS REG_IE
    // if this is not done we can get in a loop of short
    // timer irqs for example
    ldr r13, DTCM(vm_forcedIrqMask)
    mov r0, #0x04000000
    str r13, [r0, #0x210]
    ldr r0, DTCM(vm_irqSavedR0)

    mrs r13, spsr
    bic r13, r13, #0xCF
    orr r13, r13, lr
    str r13, DTCM(vm_spsr_irq)
    and r13, r13, #0xF
    mov lr, #0x92
    orrmi lr, lr, #0x40
    str lr, DTCM(vm_cpsr)

    msr spsr, #0x9F // system mode, irqs stay off, arm mode
    mov lr, #(vm_regs_irq + 3)
    add pc, pc, r13, lsl #5
    nop
old_mode_usr:
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
    nop
    nop
    nop
    nop
old_mode_fiq:
    add r13, lr, #(vm_regs_sys - vm_regs_irq)
    stmdb r13, {r8,r9,r10,r11,r12,r13,lr}^
    nop
    ldmia r13, {r8,r9,r10,r11,r12}^
    nop
    ldmia lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
    nop
old_mode_irq:
    ldmib lr, {lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_svc:
    add lr, lr, #(vm_regs_svc - vm_regs_irq)
    stmia lr, {r13,lr}^
    nop
    ldmdb lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
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
    add r13, lr, #(vm_regs_abt - vm_regs_irq)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
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
    add r13, lr, #(vm_regs_und - vm_regs_irq)
    stmia r13, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)
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
    stmdb lr, {r13,lr}^
    nop
    ldmia lr, {r13,lr}^
    adds pc, pc, #(bios_irq_handler - 8 - .)

bios_irq_handler:
    PUSH {R0-R3,R12,LR}
    MOV R3, #0x4000000
    ldr r1, [R3,#-4]
    cmp r1, #0x06800000
    ldrhs pc,= gotoBiosIrq
    ldr pc,= 2f

.text
2:
    ldr r2,= vm_emulatedIfImeIe
    ldr r12,= (emu_ioRegisters + 0x200)
    ldrh r2, [r2]
    ldrh r12, [r12]
    mov r1, r2, lsr #15
    bic r2, r2, #0xC000
    orr r2, r12, r2, lsl #16
    // game irq handler
    @ ldr r2, [r3, #0x200]!
    add r3, r3, #0x200
    @ ldrh r1, [r3, #8]
    // mrs r0, spsr
    @ push {r0-r3,lr}
    @ mov r0, #1
    @ strh r0, [r3, #8]
    and r1, r2, r2, lsr #16
    rsb r12, r1, #0
    and r0, r12, r1
    clz r12, r0
    rsb r12, r12, #31

    push {r10-r12}
    ldr r11,= vm_emulatedIfImeIe
    ldr r10, [r11]
    bic r10, r10, r0
    str r10, [r11]

    ldr r12, [r11, #(vm_emulatedIrqMask - vm_emulatedIfImeIe)]
    ldr r10, [r11, #(vm_forcedIrqMask - vm_emulatedIfImeIe)]
    ldr r11, [r11, #(vm_hwIEAddr - vm_emulatedIfImeIe)]
    bic r0, r0, r12
    bic r0, r0, r10
    strh r0, [r11, #4] // ack in DS REG_IF
    pop {r10-r12}

    msr cpsr_c, #0x10
    @ strh r0, [r3, #2]
    @ bic r2, r2, r0
    @ and r1, r2, #0xFFFFFFFE
    @ strh r1, [r3]
    // mrs r3, cpsr
    @ bic r3, r3, #0xDF
    @ orr r3, r3, #0x1F
    // msr cpsr, r3
    ldr r1,= 0x03001F80
    ldr r0, [r1, r12, lsl #2]
    blx r0
    // mrs r3, cpsr
    @ bic r3, r3, #0xDF
    @ orr r3, r3, #0x92
    // msr cpsr, r3
    @ pop {r0-r3,lr}
    @ strh r2, [r3]
    @ strh r1, [r3, #8]
    // msr spsr, r0
loc_138:
    POP {R0-R3,R12,LR}
    vmSUBS pc, lr, 4

gotoBiosIrq:
    adr lr, loc_138
    msr cpsr_c, #0x10
    mov pc, r1
