#pragma once
#include "Context.h"

extern "C" void test_runArmInstruction(u32 instruction, const context_t* inContext, context_t* outContext);
extern "C" bool test_isArmInstructionUndefined(u32 instruction, const context_t* inContext, context_t* outContext);
