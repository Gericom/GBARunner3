.section ".itcm", "ax"
.arm

vec_reset:
    b .
vec_undef:
    b .
vec_swi:
    b .
vec_prefetchAbort:
    b memu_prefetchAbort
vec_dataAbort:
    mrs r13, spsr
    // r13 = ???? ?000 0000 0000 0000 0000 00T1 0000
    mov pc, r13, lsl #6 // arm -> 0x400, thumb -> 0xC00
vec_irq:
    b .
vec_fiq:
    b .
