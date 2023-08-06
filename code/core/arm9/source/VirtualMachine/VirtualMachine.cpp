#include "common.h"
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
    vm_hwIrqMask = 0;
    vm_emulatedIfImeIe = 0;
    return vm_run(_startAddress, context, &_storeContext);
}
