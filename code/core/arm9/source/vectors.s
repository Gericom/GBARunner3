.section ".itcm", "ax"
.arm

vec_reset:
    b .
vec_undef:
    b .
vec_swi:
    b .
vec_prefetchAbort:
    b .
vec_dataAbort:
    mrs r13, spsr
    // r13 = ???? ?000 0000 0000 0000 0000 00T1 0000
    mov pc, r13, lsl #5 // arm -> 0x200, thumb -> 0x600
vec_irq:
    b .
vec_fiq:
    b .
