.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "../MemoryEmuDtcm.inc"

arm_dispatch_base:

#define DTCM(x) #(arm_dispatch_base - 0x900 + x)

arm_func memu_armDispatch
    str lr, DTCM(memu_inst_addr)
    msr cpsr_c, #0xD1 // switch to fiq mode
    ldr r11, DTCM(memu_inst_addr)
    // interlock
    ldr lr, [r11, #-8] // lr = instruction
    ldr r13, DTCM(memu_arm_table_addr)
    and r8, lr, #0x0FF00000
    tst lr, #0x0E000000

    and r9, lr, #0xE0
    orreq r8, r9, lsl #20
    add r8, r13, r8, lsr #16
    ldmia r8, {r8, r10, r11}
    // Rm
    and r9, lr, #0xF
    ldr r12, [r8, r9, lsl #2]
    // Rn
    and r8, lr, #(0xF << 16)
    ldr r8, [r10, r8, lsr #14]
    // Rd
    and r10, lr, #(0xF << 12)
    ldr r11, [r11, r10, lsr #10]

    bx r12

.end
