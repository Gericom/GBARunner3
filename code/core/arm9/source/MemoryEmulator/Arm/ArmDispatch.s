.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_dispatch_base:

#define DTCM(x) #(arm_dispatch_base - 0x400 + x)

arm_func memu_armDispatch
    str lr, DTCM(memu_inst_addr)
    msr cpsr_c, #0x91 // switch to fiq mode
    ldr r11, DTCM(memu_inst_addr)
    ldr r8, DTCM(memu_arm_table_addr)

#ifdef GBAR3_HICODE_CACHE_MAPPING
    cmp r11, #0x08000000
        bhs readInstructionFromCache
#endif

    ldr lr, [r11, #-8] // lr = instruction

armDispatchContinue:
    movs r10, #(0xF << 2) // also clears Z flag
    and r12, lr, #0x0FF00000
    mov r9, lr, lsl #25
    add r11, r8, r9, lsr #30
    ldrb r12, [r11, r12, lsr #18]!
    ldrb r8, [r11, #0x500]
    // Rn
    and r9, r10, lr, lsr #14
    ldr r12, [r9, -r12, lsl #6] // r12 = Rn handler address
    // Rm
    and r9, r10, lr, lsl #2 // r9 = Rm << 2
    ldr r8, [r9, -r8, lsl #6] // r8 = Rm handler address
    // Rd
    ldrb r11, [r11, #0x280]
    bx r8

#ifdef GBAR3_HICODE_CACHE_MAPPING

readInstructionFromCache:
    sub r11, r11, #8
    bic r11, r11, #0xFE000000
    tst r11, #0x800
    orrne r11, r11, #0x40000000 // set
    mcr p15, 3, r11, c15, c0, 0 // set index
    mrc p15, 3, lr, c15, c3, 0 // read data
    b armDispatchContinue

#endif

.end
