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
    // r13 = ???? ?000 0000 0000 0000 0000 10T1 0000
    // assume Q flag is 0
    mov pc, r13, lsl #4 // arm -> 0x900, thumb -> 0xB00
vec_irq:
    b .
vec_fiq:
    LDR     SP, =0x27FFD9C
    ADD     SP, SP, #1
    PUSH    {R12,LR}
    MRS     LR, SPSR
    MRC     p15, 0, R12,c1,c0, 0
    PUSH    {R12,LR}
    BIC     R12, R12, #1
    MCR     p15, 0, R12,c1,c0, 0
    BIC     R12, SP, #1
    LDR     R12, [R12,#0x10]
    CMP     R12, #0
    BLXNE   R12
    POP     {R12,LR}
    MCR     p15, 0, R12,c1,c0, 0
    MSR     SPSR_cxsf, LR
    POP     {R12,LR}
    SUBS    PC, LR, #4

.end
