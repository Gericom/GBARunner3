.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"
#include "MemoryEmuDtcm.inc"

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
    b badPrefetchAbort
#endif
