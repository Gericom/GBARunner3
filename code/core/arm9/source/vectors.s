.section ".itcm", "ax"
.arm

vec_reset:
    b .
vec_undef:
    b .
vec_swi:
    b .
vec_prefetchAbort:
    b prefetchAbort
vec_dataAbort:
    mrs r13, spsr
    // r13 = ???? ?000 0000 0000 0000 0000 00T1 0000
    mov pc, r13, lsl #5 // arm -> 0x200, thumb -> 0x600
vec_irq:
    b .
vec_fiq:
    b .

prefetchAbort:
    sub lr, lr, #4
    sub lr, lr, #0x08000000
    cmp lr, #0x02000000
    bhs badPrefetchAbort
    bic lr, lr, #0xFE000000
    adds pc, lr, #0x02200000

badPrefetchAbort:
    b .
