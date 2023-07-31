.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

vm_irq_base:

#define DTCM(x) (vm_irq_base - 0x100 + (x))

arm_func vm_irq
    ldr r13, DTCM(vm_hwIEAddr)
    str r8, DTCM(vm_irqSavedR8)
    str r9, DTCM(vm_irqSavedR9)
    str lr, DTCM(vm_irqSavedLR)
    // here emulator irq tasks can be performed

    // todo: Don't tigger hblank irq on scanlines beyond the gba screen

    ldmia r13, {r8,r9} // r8 = IE, r9 = IF
    ldr lr, DTCM(vm_hwIrqMask)
    and r8, r8, r9
    str r8, [r13, #4] // ack in hardware IF
    ldr r13, DTCM(vm_emulatedIF) // and IE in the top 14 bits, and IME top most bit
    and lr, lr, r9 // bits to add to the emulated IF
    orr r13, r13, lr
    str r13, DTCM(vm_emulatedIF)
    
    tst r8, #(1 << 16) // ARM7 IRQ
    blne emu_arm7Irq
    tst r8, #1 // VBLANK IRQ
    blne emu_vblankIrq

    ldr r13, DTCM(vm_emulatedIF)
    ldr r8, DTCM(vm_irqSavedR8)
    ldr r9, DTCM(vm_irqSavedR9)
    ldr lr, DTCM(vm_irqSavedLR)
    and r13, r13, r13, lsl #16 // IF & IE
    tst r13, r13, asr #16 // (IF & IE) & IME
    ldr r13, DTCM(vm_cpsr)
    subeqs pc, lr, #4 // no irq available for the vm

    teq r13, r13, lsl #24 // C = I flag, N = F flag
    subcss pc, lr, #4 // vm doesn't want irqs; nothing changed; resume where we left off

    mrs lr, spsr
    bic lr, lr, #0xCF
    orr lr, lr, r13, lsr #1
    str lr, DTCM(vm_spsr_irq)
    ldr r13, DTCM(vm_toIrqSwitchTableAddr)
    and lr, lr, #(0xF << 1)
    ldrh lr, [r13, lr]
    mov r13, #(0x92 << 1)
    orrmi r13, r13, #(0x40 << 1)
    str r13, DTCM(vm_cpsr)
    adr r13, 1f
    bx lr // call the mode switch function
1:
    adr r13, DTCM(vm_irqSavedLR)
    ldmia r13, {lr}^
    nop

    ldr lr, DTCM(vm_irqVector)
    msr spsr, #0x10 // user mode, irqs stay on, arm mode
    movs pc, lr
