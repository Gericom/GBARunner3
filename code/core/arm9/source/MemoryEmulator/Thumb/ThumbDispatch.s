.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "../MemoryEmuDtcm.inc"

thumb_dispatch_base:

#define DTCM(x) #(thumb_dispatch_base - 0xB00 + x)

arm_func memu_thumbDispatch
    str lr, DTCM(memu_inst_addr)
    msr cpsr_c, #0xD1 // switch to fiq mode
    ldr lr, DTCM(memu_inst_addr)
    // interlock
    ldrh lr, [lr, #-8] // lr = instruction
    mvn r9, #0xFF000 // mask for dtcm mirroring
    ldr r13, DTCM(memu_thumb_table_addr)

    orr r8, lr, lr, lsl #16
    and r8, r9, r8, lsr #7 // dtcm must mirror

    and r9, lr, #0x1F8 // r9 = (Rm_Rn) << 3
    ldr r9, [r13, -r9, lsr #1] // r9 = address of Rm and Rn resolving function

    ldrh r8, [r13, r8] // r8 = address of instruction handler

    bx r9

.end
