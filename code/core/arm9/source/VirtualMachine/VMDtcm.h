#pragma once

extern void* vm_irqVector;
extern void* vm_swiVector;
extern u32 vm_cpsr;
extern u32 vm_spsr_fiq;
extern u32 vm_spsr_irq;
extern u32 vm_spsr_svc;
extern u32 vm_spsr_abt;
extern u32 vm_spsr_und;
