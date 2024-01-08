#include "common.h"
#include <libtwl/rtos/rtosIrq.h>
#include <string.h>
#include "VMDtcm.h"
#include "VirtualMachine.h"

extern "C" u32 vm_run(void* startAddress, const context_t* context, context_t* storeContext);

u32 VirtualMachine::Run(const context_t* context)
{
    vm_irqVector = _irqVector;
    vm_swiVector = _swiVector;
    vm_cpsr = 0xD3; // I = 1, F = 1, ARM, SVC mode
    vm_spsr_fiq = 0;
    vm_spsr_irq = 0;
    vm_spsr_svc = 0;
    vm_spsr_abt = 0;
    vm_spsr_und = 0;
    memset(vm_regs_fiq, 0, sizeof(vm_regs_fiq));
    memset(vm_regs_sys, 0, sizeof(vm_regs_sys));
    memset(vm_regs_irq, 0, sizeof(vm_regs_irq));
    memset(vm_regs_svc, 0, sizeof(vm_regs_svc));
    memset(vm_regs_abt, 0, sizeof(vm_regs_abt));
    memset(vm_regs_und, 0, sizeof(vm_regs_und));
    vm_hwIrqMask = 0;
    vm_emulatedIfImeIe = 0;
    vm_forcedIrqMask = RTOS_IRQ_GX_FIFO | RTOS_IRQ_IPC_SYNC | RTOS_IRQ_VBLANK;
    return vm_run(_startAddress, context, &_storeContext);
}
