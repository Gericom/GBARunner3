.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "SdCache/SdCacheDefs.h"

#ifdef GBAR3_HICODE_CACHE_MAPPING

/// @brief Unmaps the currently mapped rom block (if any).
/// @param r0 Trashed
/// @param lr Return address
arm_func hic_unmapRomBlock
    mov r0, #0
    mcr p15, 0, r0, c9, c0, 1 // unlock icache
    mcr	p15, 0, r0, c6, c4, 0 // disable mpu region
    bx lr

/// @brief Maps the 4 kB rom block at the given address into the instruction cache.
/// @param r0 The GBA rom address 0x08000000-0x0DFFFFFF
/// @param sp Stack pointer
/// @param lr Return address
arm_func hic_mapRomBlock
    push {r4, lr}
    bic r4, r0, #0x06000000
    mov r4, r4, lsr #12
    mov r4, r4, lsl #12
    ldr r1,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    //setup the pu region
    orr r12, r4, #0x17
    ldr r0, [r1, r4, lsr #(SDC_BLOCK_SHIFT - 2)]
    mcr	p15, 0, r12, c6, c4, 0
    
    cmp r0, #0
    moveq r0, r4
    bleq sdc_loadRomBlockDirect

    orr r1, r4, #(1 << 4) //valid flag

    mov r12, #0x80000000 //load bit + segment 0
    mcr p15, 0, r12, c9, c0, 1

prefetchCacheSet0:
    mov r12, #64
1:
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    mcr p15, 3, r0, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    mcr p15, 3, r0, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    mcr p15, 3, r0, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    mcr p15, 3, r0, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r0, r0, #32
    subs r12, r12, #4
    bne 1b

    add r1, r1, #2048
    mov r12, #0x80000001 //load bit + segment 1
    mcr p15, 0, r12, c9, c0, 1
    orr r3, r1, #(1 << 30)

prefetchCacheSet1:
    mov r12, #64
2:
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    add r0, r0, #32
    mcr p15, 0, r0, c7, c13, 1 //prefetch
    add r0, r0, #32

    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r3, r3, #32
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r3, r3, #32
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r3, r3, #32
    mcr p15, 3, r3, c15, c0, 0 //set index
    mcr p15, 3, r1, c15, c1, 0 //write tag
    add r3, r3, #32

    subs r12, r12, #4
    bne 2b

    mov r0, #0x00000002 //segment 2
    mcr p15, 0, r0, c9, c0, 1
    pop {r4, pc}

#endif
