.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"
#include "VMUndefinedInstructions.inc"

vm_irq_base:

#define DTCM(x) (vm_irq_base - 0xB8 + (x))

/// @brief Main irq handler that handles both emulation irqs and passed
///        through irqs to the VM. Should be branched to directly from
///        the hardware irq vector.
/// @param lr The return address + 4.
arm_func vm_irq
    str r4, DTCM(vm_irqSavedR4)
    // here emulator irq tasks can be performed

    mov r13, #0x04000000
    ldr r4, [r13, #0x214]
    str lr, DTCM(vm_irqSavedLR)
    str r4, [r13, #0x214]

    tst r4, #(1 << 16) // ARM7 IRQ
        bne emu_arm7Irq
arm_func emu_arm7IrqReturn
    tst r4, #2 // HBLANK IRQ
        bne emu_hblankIrq
arm_func emu_hblankIrqReturn
    tst r4, #1 // VBLANK IRQ
        bne emu_vblankIrq
arm_func emu_vblankIrqReturn

    ldr r13, DTCM(vm_hwIrqMask)
    ldr lr, DTCM(vm_emulatedIfImeIe)
    and r13, r13, r4 // bits to add to the emulated IF
    orr r13, lr, r13
    str r13, DTCM(vm_emulatedIfImeIe)

    ldr r4, DTCM(vm_irqSavedR4)
    // 0EEE EEEE EEEE EEE0 M0FF FFFF FFFF FFFF (E = IE, M = IME, F = IF)
    //M0FFF FFFF FFFF FFF0
    tst r13, r13, lsl #17 // IF & IE
    ldr lr, DTCM(vm_irqSavedLR)
    ldr r13, DTCM(vm_cpsr)
.global vm_irqReturnForNestedIrq
vm_irqReturnForNestedIrq:
    sublss pc, lr, #4 // IME = 0 or (IF & IE == 0)

    teq r13, r13, lsl #25 // C = I flag, N = F flag
    subcss pc, lr, #4 // vm doesn't want irqs; nothing changed; resume where we left off

    mrs r4, spsr
    bic r4, r4, #0xCF
    orr r4, r4, r13
    ldr sp, DTCM(vm_regs_irq) // sp_irq
    str r4, DTCM(vm_spsr_irq)

    stmfd sp!, {r0-r3,r12,lr}
    ldr r1, DTCM(vm_returnFromIrqAddress)
    str sp, DTCM(vm_regs_irq)
    str r1, DTCM(vm_regs_irq + 4)

    // only forced irqs stay on in DS REG_IE
    // if this is not done we can get in a loop of short
    // timer irqs for example
    ldr r1, DTCM(vm_forcedIrqMask)
    mov r0, #0x04000000
    strb r0, DTCM(memu_biosOpcodeId) // store MEMU_BIOS_OPCODE_ID_IRQ_ENTRY
    str r1, [r0, #0x210]

    mov lr, #0x92
    orrmi lr, lr, #0x40
    str lr, DTCM(vm_cpsr)

    and r1, r4, #0xF
    ldr r4, [r0, #-4] // load irq handler address

    mov lr, #(vm_regs_irq + 3)
    add pc, pc, r1, lsl #5
    nop
old_mode_usr:
    stmdb lr, {r13,lr}^
    b jumpToIrqHandlerWithLdmiaLr
    nop
    nop
    nop
    nop
    nop
    nop
old_mode_fiq:
    add r1, lr, #(vm_regs_sys - vm_regs_irq)
    stmdb r1, {r8,r9,r10,r11,r12,r13,lr}^
    nop
    ldmia r1, {r8,r9,r10,r11,r12}^
    b jumpToIrqHandlerWithLdmiaLr
    nop
    nop
    nop
old_mode_irq:
    ldmib lr, {lr}^
    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    b vm_jumpToIrqHandler
    nop
    nop
    nop
    nop
    nop
old_mode_svc:
    add lr, lr, #(vm_regs_svc - vm_regs_irq)
    stmia lr, {r13,lr}^
    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    ldmdb lr, {r13,lr}^
    b vm_jumpToIrqHandler
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
    add r1, lr, #(vm_regs_abt - vm_regs_irq)
    stmia r1, {r13,lr}^
    b jumpToIrqHandlerWithLdmiaLr
    nop
    nop
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
    add r1, lr, #(vm_regs_und - vm_regs_irq)
    stmia r1, {r13,lr}^
    b jumpToIrqHandlerWithLdmiaLr
    nop
    nop
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
jumpToIrqHandlerWithLdmiaLr:
    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    ldmia lr, {r13,lr}^

arm_func vm_jumpToIrqHandler
// this part is only needed when jit is on
// when modifying keep jit_disable in mind
    mov r0, r4
#ifndef GBAR3_TEST
    bl jit_ensureBlockJitted
#endif

arm_func vm_jumpToIrqHandlerCommon
    mov r0, #0x04000000 // not strictly necessary when jit is off, but safety for ldmia^
    mov lr, r4
    ldr r4, DTCM(vm_irqSavedR4)
    movs pc, lr

.text
arm_func vm_returnFromIrq
    pop {r0-r3,r12,lr}
    vmSUBS pc, lr, 4
