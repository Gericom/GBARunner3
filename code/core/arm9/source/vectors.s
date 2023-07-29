.section ".itcm.vectors", "ax"
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
    // r13 = ???? ?000 0000 0000 0000 0000 10T1 0000
    // assume Q flag is 0
    mov pc, r13, lsl #4 // arm -> 0x900, thumb -> 0xB00
vec_irq:
    b .
vec_fiq:
    b .
