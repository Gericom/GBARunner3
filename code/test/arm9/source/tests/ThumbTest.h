#pragma once
#include "Context.h"

extern "C" void test_runThumbInstruction(u16 instruction, const context_t* inContext, context_t* outContext);
extern "C" bool test_isThumbInstructionUndefined(u16 instruction, const context_t* inContext, context_t* outContext);
