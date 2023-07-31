#pragma once

#define VM_DTCM_BASE 0xFFFFFE00

.struct VM_DTCM_BASE
vm_irqSavedR8:
    .word 0
vm_irqSavedR9:
    .word 0
vm_irqSavedLR:
    .word 0
vm_irqVector:
    .word 0
vm_toIrqSwitchTableAddr:
    .word 0
vm_hwIEAddr:
    .word 0  // Address of DS REG_IE; 0x04000210
vm_hwIrqMask:
    .word 0  // DS irq mask indicating which bits of IF should be passed through to the VM
             // This is related to the underlying sources being enabled or not.
             // For example whether irq bits in the emulated REG_DISPSTAT are enabled.
             // It is very well possible that the DS REG_DISPSTAT has the VBLANK irq enabled
             // while the emulated REG_DISPSTAT has not. In that case the VBLANK bit in the
             // emulated REG_IF should not become set.
vm_emulatedIF:
    .short 0 // Emulated REG_IF presented to the GBA game
vm_emulatedIE:
    .short 0 // Emulated REG_IE

vm_undefinedInstructionAddr:
    .word 0
vm_undefinedSpsr:
    .word 0
vm_undefinedRegTmp:
    .word 0
vm_undefinedArmTableAddr:
    .word 0

vm_cpsr:
    .word 0
vm_spsr:
    .word 0
vm_spsr_fiq:
    .word 0
vm_spsr_irq:
    .word 0
vm_spsr_svc:
    .word 0
    .word 0
    .word 0
    .word 0
vm_spsr_abt:
    .word 0
    .word 0
    .word 0
    .word 0
vm_spsr_und:
    .word 0

vm_regs_fiq:
    .word 0 // r8
    .word 0 // r9
    .word 0 // r10
    .word 0 // r11
    .word 0 // r12
    .word 0 // sp
    .word 0 // lr

vm_regs_sys:
    .word 0 // r8
    .word 0 // r9
    .word 0 // r10
    .word 0 // r11
    .word 0 // r12
    .word 0 // sp
    .word 0 // lr

vm_regs_irq:
    .word 0 // sp
    .word 0 // lr
    
vm_regs_svc:
    .word 0 // sp
    .word 0 // lr

vm_regs_abt:
    .word 0 // sp
    .word 0 // lr

vm_regs_und:
    .word 0 // sp
    .word 0 // lr

.previous
