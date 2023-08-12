#pragma once

extern void* vm_irqVector;
extern void* vm_swiVector;
extern u32 vm_hwIrqMask;
extern u32 vm_emulatedIfImeIe;
extern u32 vm_cpsr;
extern u32 vm_spsr_fiq;
extern u32 vm_spsr_irq;
extern u32 vm_spsr_svc;
extern u32 vm_spsr_abt;
extern u32 vm_spsr_und;
extern u32 vm_regs_fiq[7];
extern u32 vm_regs_sys[7];
extern u32 vm_regs_irq[2];
extern u32 vm_regs_svc[2];
extern u32 vm_regs_abt[2];
extern u32 vm_regs_und[2];
