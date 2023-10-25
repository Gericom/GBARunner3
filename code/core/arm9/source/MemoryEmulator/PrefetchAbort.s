.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "SdCache/SdCacheDefs.h"

arm_func memu_prefetchAbort
    sub lr, lr, #4
    cmp lr, #0x03000000
        blo prefetchAbortRomRelative
    cmp lr, #0x0C000000
        blo prefetchAbortRomAbsolute
badPrefetchAbort:
    b .

prefetchAbortRomRelative:
    add lr, lr, #(0x08000000 - 0x02200000)

prefetchAbortRomAbsolute:
    cmp lr, #0x08200000
        addlos pc, lr, #(0x02200000 - 0x08000000)

#ifdef GBAR3_HICODE_CACHE_MAPPING
    ldr sp,= dtcmStackEnd
    push {r0-r3, r12, lr}
    mov r0, lr
    bl hic_mapRomBlock
    ldmfd sp, {r0-r3, r12, pc}^
#else
    ldr sp,= dtcmStackEnd
    push {r0, r4}
    ldr r0,= (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
    bic r4, lr, #(3 << (SDC_BLOCK_SHIFT - 2))
    ldr r0, [r0, r4, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r4, lr, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r0, #0
        addne lr, r0, r4, lsr #(32 - SDC_BLOCK_SHIFT)
        popne {r0, r4}
        movnes pc, lr

    push {r1, r2, r3, r12}
    mov r0, lr
    bl sdc_loadRomBlockDirect

    add r0, r0, r4, lsr #(32 - SDC_BLOCK_SHIFT)
    mov r4, r0
    mrs r1, spsr
    tst r1, #0x20
        orrne r0, r0, #1
    bl jit_ensureBlockJitted
    mov lr, r4
    pop {r1, r2, r3, r12}
    pop {r0, r4}
    movs pc, lr
#endif
