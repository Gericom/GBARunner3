#pragma once
#include "Context.h"

class VirtualMachine
{
    void* _startAddress;
    void* _swiVector;
    void* _irqVector;
    context_t _storeContext;
public:
    VirtualMachine(void* startAddress, void* swiVector, void* irqVector)
        : _startAddress(startAddress), _swiVector(swiVector), _irqVector(irqVector) { }

    u32 Run(const context_t* context);
};
