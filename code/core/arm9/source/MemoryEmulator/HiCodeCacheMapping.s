.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "SdCache/SdCacheDefs.h"
#include "HiCodeCacheMappingDefs.h"

#ifdef GBAR3_HICODE_CACHE_MAPPING

/// @brief Unmaps the currently mapped rom block (if any).
/// @param r0 Trashed
/// @param lr Return address
arm_func hic_unmapRomBlock
    mov r0, #0
    mcr	p15, 0, r0, c6, c4, 0 // disable mpu region
    bx lr

arm_func hic_undefinedHicodeMiss
    tst r13, #0x20 // spsr thumb bit
    ldr sp,= dtcmHicodeStackEnd
    push {r3,r12}
    ldr r12,= gHicodeState
        subeq r3, lr, #4 // arm
    ldr r12, [r12]
        subne r3, lr, #2 // thumb
    mcr p15, 3, r3, c15, c0, 0 // set index
    eor r12, r12, r3
    cmp r12, #0x800
    bcs 1f // cache segment 1 -> always hicode miss

    ldr r3,= HICODE_UNDEFINED_INSTRUCTION
    mrc p15, 3, r12, c15, c3, 0 // read data
    cmp r12, r3
    bne notHicodeMiss
1:
    pop {r3,r12}
    mrc p15, 3, lr, c15, c0, 0 // get index (=instruction address)
    push {r0-r3,r12,lr}
    mov r0, lr
    bl hic_mapRomBlock
    ldmfd sp, {r0-r3,r12,pc}^

notHicodeMiss:
    // this is not a hicode miss, but a regular undefined instruction
    pop {r3,r12}
    ldr r13,= vm_undefinedInstructionAddr
    str lr, [r13]
    msr cpsr_c, #0xD1 // switch to fiq mode
    ldr r8,= vm_undefinedInstructionAddr
    mrc p15, 3, lr, c15, c3, 0 // read data
    ldr r11, [r8]
    b vm_undefinedArmInstructionInLR

#endif


