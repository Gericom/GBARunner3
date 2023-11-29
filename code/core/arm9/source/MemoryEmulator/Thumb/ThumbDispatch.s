.section ".itcm", "ax"
.altmacro
.arm

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

thumb_dispatch_base:

#define DTCM(x) #(thumb_dispatch_base - 0xC00 + x)

arm_func memu_thumbDispatch
    str r0, DTCM(memu_thumb_r0)

#ifdef GBAR3_HICODE_CACHE_MAPPING
    cmp lr, #0x08000000
        bhs readInstructionFromCache
#endif

    ldrh r0, [lr, #-8]

thumbDispatchContinue:
    msr cpsr_c, #0xD1 // switch to fiq mode

    orr r11, r0, r0, lsl #16
    mvn r9, #0x3000 // mask for dtcm mirroring; 0xFFFFCFFF
    bic r12, r9, r11, lsr #7 // dtcm must mirror
    ldrh r12, [r12] // r12 = address of instruction handler
    ldr r0, DTCM(memu_thumb_r0)
    and r8, r11, #0x1F8 // r8 = (Rm_Rn) << 3
    add pc, r8, r12, lsl #12

#ifdef GBAR3_HICODE_CACHE_MAPPING

readInstructionFromCache:
    ldr r0,= memu_inst_addr
    str lr, [r0]
    sub r0, lr, #8
    bic r0, r0, #0xFE000000
    tst r0, #0x800
    orrne r0, r0, #0x40000000 // set
    mcr p15, 3, r0, c15, c0, 0 // set index
    mrc p15, 3, r0, c15, c3, 0 // read data
    tst lr, #2
    moveq r0, r0, lsl #16
    mov r0, r0, lsr #16
    b thumbDispatchContinue

#endif

.end
