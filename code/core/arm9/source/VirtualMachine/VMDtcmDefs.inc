#pragma once

.global vm_thumbUndefinedDispatchTable
.set vm_thumbUndefinedDispatchTable, 0xFFFFF32C

.global vm_armUndefinedDispatchTable
.set vm_armUndefinedDispatchTable, 0xFFFFF72C

#define VM_DTCM_BASE 0xFFFFFF2C

.struct VM_DTCM_BASE
vm_irqSavedR0:
    .word 0
vm_irqSavedLR:
    .word 0
.global vm_irqVector
vm_irqVector:
    .word 0
.global vm_swiVector
vm_swiVector:
    .word 0
vm_hwIEAddr:
    .word 0  // Address of DS REG_IE; 0x04000210

// DS irq mask indicating which bits of IF should be passed through to the VM
// This is related to the underlying sources being enabled or not.
// For example whether irq bits in the emulated REG_DISPSTAT are enabled.
// It is very well possible that the DS REG_DISPSTAT has the VBLANK irq enabled
// while the emulated REG_DISPSTAT has not. In that case the VBLANK bit in the
// emulated REG_IF should not become set.
.global vm_hwIrqMask
vm_hwIrqMask:
    .word 0  

// DS irq mask indicating which irqs are force enabled because they are
// required for the emulation
.global vm_forcedIrqMask
vm_forcedIrqMask:
    .word 0

// GBA irq mask indicating which irqs don't originate from a DS hw source
.global vm_emulatedIrqMask
vm_emulatedIrqMask:
    .word 0

.global vm_emulatedIfImeIe
vm_emulatedIfImeIe:
    .word 0 // 0EEE EEEE EEEE EEE0 M0FF FFFF FFFF FFFF (E = IE, M = IME, F = IF)

vm_undefinedInstructionAddr:
    .word 0
vm_undefinedSpsr:
    .word 0
vm_undefinedRegTmp:
    .word 0
vm_undefinedArmTableAddr:
    .word 0
vm_undefinedThumbTableAddr:
    .word 0
vm_svcRegsAddr:
    .word 0
vm_modeSwitchTableAddr:
    .word 0

.global vm_cpsr
vm_cpsr:
    .word 0
vm_spsr:
    .word 0
.global vm_spsr_fiq
vm_spsr_fiq:
    .word 0
.global vm_spsr_irq
vm_spsr_irq:
    .word 0
.global vm_spsr_svc
vm_spsr_svc:
    .word 0

.global memu_inst_addr
memu_inst_addr:
    .word 0
.global memu_thumb_r0
memu_thumb_r0:
    .word 0
.global memu_thumb_table_addr
memu_thumb_table_addr:
    .word 0

.global vm_spsr_abt
vm_spsr_abt:
    .word 0

.global memu_arm_table_addr
memu_arm_table_addr:
    .word 0

.global dma_transferRegister
dma_transferRegister:
    .word 0
.global dma_stepTable
dma_stepTable:
    .word 0
.global vm_spsr_und
vm_spsr_und:
    .word 0

.global vm_regs_fiq
vm_regs_fiq:
    .word 0 // r8
    .word 0 // r9
    .word 0 // r10
    .word 0 // r11
    .word 0 // r12
    .word 0 // sp
    .word 0 // lr

.global vm_regs_sys
vm_regs_sys:
    .word 0 // r8
    .word 0 // r9
    .word 0 // r10
    .word 0 // r11
    .word 0 // r12
    .word 0 // sp
    .word 0 // lr

.global vm_regs_irq
vm_regs_irq:
    .word 0 // sp
    .word 0 // lr

.global vm_regs_svc
vm_regs_svc:
    .word 0 // sp
    .word 0 // lr

.global vm_regs_abt
vm_regs_abt:
    .word 0 // sp
    .word 0 // lr

.global vm_regs_und
vm_regs_und:
    .word 0 // sp
    .word 0 // lr

#define MEMU_BIOS_OPCODE_ID_IRQ_ENTRY   0
#define MEMU_BIOS_OPCODE_ID_RESET       1
#define MEMU_BIOS_OPCODE_ID_IRQ_EXIT    2
#define MEMU_BIOS_OPCODE_ID_SWI_EXIT    3

.global memu_biosOpcodeId
memu_biosOpcodeId:
    .word 0

.global memu_adjustedRomBlockToCacheBlockAddress
memu_adjustedRomBlockToCacheBlockAddress:
    .word 0

.previous
