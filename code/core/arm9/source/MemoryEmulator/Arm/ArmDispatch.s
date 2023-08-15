.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "../MemoryEmuDtcm.inc"

arm_dispatch_base:

#define DTCM(x) #(arm_dispatch_base - 0x400 + x)

arm_func memu_armDispatch
    str lr, DTCM(memu_inst_addr)
    msr cpsr_c, #0x91 // switch to fiq mode
    ldr r11, DTCM(memu_inst_addr)
    ldr r12, DTCM(memu_arm_table_addr)
    ldr lr, [r11, #-8] // lr = instruction
    mov r13, r12
    and r8, lr, #0x0FF00000
    tst lr, #0x0E000000

    and r9, lr, #0xE0
    orreq r8, r9, lsl #20
    ldr r8, [r12, r8, lsr #17]!
    ldrsh r10, [r12, #4]
    ldrsh r11, [r12, #6]
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
