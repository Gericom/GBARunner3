.section ".itcm", "ax"
.arm

#define REG_SCFG_CLK        0x04004004
#define SCFG_CLK_CPU_SPEED  1

.global scfg_setArm9Clock
.type scfg_setArm9Clock, %function
scfg_setArm9Clock:
    ldr r3,= REG_SCFG_CLK
    ldrh r2, [r3]
    and r1, r2, #SCFG_CLK_CPU_SPEED
    cmp r1, r0
        bxeq lr // requested speed already set

    mrs r12, cpsr
    orr r1, r12, #0xC0 // disable irq and fiq
    msr cpsr, r1

    bic r2, r2, #SCFG_CLK_CPU_SPEED
    orr r2, r2, r0

    strh r2, [r3]

    // allow the clock switch to stabilize
    mov r0, #8
1:
    subs r0, r0, #1
    bne 1b

    msr cpsr, r12
    bx lr