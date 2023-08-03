#include "common.h"
#include "VirtualMachine.h"

extern "C" u32 vm_run(void* startAddress, const context_t* context, context_t* storeContext);

extern void* vm_irqVector;
extern void* vm_swiVector;
extern u32 vm_cpsr;

u32 VirtualMachine::Run(const context_t* context)
{
    vm_irqVector = _irqVector;
    vm_swiVector = _swiVector;
    vm_cpsr = 0xD3; // I = 1, F = 1, ARM, SVC mode
    return vm_run(_startAddress, context, &_storeContext);
}
