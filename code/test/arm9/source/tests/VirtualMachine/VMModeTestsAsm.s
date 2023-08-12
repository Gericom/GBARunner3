.text
.altmacro
#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "VirtualMachine/VMUndefinedInstructions.inc"

// r0 = old mode
// r1 = new mode
// r2 = old regs in
// r3 = old regs out
// r4 = new regs in
// r5 = new regs out
arm_func VMModeTests_SwitchToMode
    mov r6, lr
    orr r0, r0, #0xD0
    orr r1, r1, #0xD0
    vmMSR cpsr_c, r0 // to old mode
    ldmia r2, {r8-lr} // set old regs
    vmMSR cpsr_c, r1 // to new mode
    ldmia r4, {r8-lr} // set new regs
    vmMSR cpsr_c, r0 // to old mode
    stmia r3, {r8-lr} // get old regs
    vmMSR cpsr_c, r1 // to new mode
    stmia r5, {r8-lr} // get new regs
    bx r6

