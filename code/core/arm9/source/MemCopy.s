.section ".itcm", "ax"

#include "AsmMacros.inc"

// r0 = src (32 bit aligned)
// r1 = dst (32 bit aligned)
// r2 = byte count (multiple of 4)
arm_func mem_copy32
    push {r4-r8,lr}
1:
    subs r2, r2, #0x20
    ldmhsia r0!, {r3-r8,r12,lr}
    stmhsia r1!, {r3-r8,r12,lr}
    bhi 1b
    popeq {r4-r8,pc}
    add r2, r2, #0x20
2:
    ldr r3, [r0], #4
    subs r2, r2, #4
    str r3, [r1], #4
    bne 2b
    pop {r4-r8,pc}

// r0 = src (16 bit aligned)
// r1 = dst (16 bit aligned)
// r2 = byte count (multiple of 2)
arm_func mem_copy16
    // cases
    // - both are 32 bit aligned
    // - src 16 bit aligned, dst 32 bit aligned
    // - dst 32 bit aligned, src 16 bit aligned
    // - both are 16 bit aligned
    and r12, r1, #2
    tst r0, #2
    orrne r12, r12, #1
    ldr pc, [pc, r12, lsl #2]
    nop
    .word copy16_src32_dst32
    .word copy16_src32_dst16
    .word copy16_src16_dst32
    .word copy16_src16_dst16

copy16_src16_dst16:
    ldrh r12, [r0], #2
    subs r2, r2, #2
    strh r12, [r1], #2
    bxeq lr

copy16_src32_dst32:
    push {r4-r8,lr}
1:
    subs r2, r2, #0x20
    ldmhsia r0!, {r3-r8,r12,lr}
    stmhsia r1!, {r3-r8,r12,lr}
    bhi 1b
    popeq {r4-r8,pc}
    add r2, r2, #0x20
2:
    ldrh r3, [r0], #2
    subs r2, r2, #2
    strh r3, [r1], #2
    bne 2b
    pop {r4-r8,pc}


copy16_src16_dst32:
    ldrh r12, [r0], #2
    subs r2, r2, #2
    strh r12, [r1], #2
    bxeq lr

copy16_src32_dst16:
    push {r4-r8,lr}
1:
    subs r2, r2, #0x20
    blo 2f
    ldmia r0!, {r3-r8,r12,lr}
    strh r3, [r1], #2
    mov r3, r3, lsr #16
    orr r3, r3, r4, lsl #16
    mov r4, r4, lsr #16
    orr r4, r4, r5, lsl #16
    mov r5, r5, lsr #16
    orr r5, r5, r6, lsl #16
    mov r6, r6, lsr #16
    orr r6, r6, r7, lsl #16
    mov r7, r7, lsr #16
    orr r7, r7, r8, lsl #16
    mov r8, r8, lsr #16
    orr r8, r8, r12, lsl #16
    mov r12, r12, lsr #16
    orr r12, r12, lr, lsl #16
    mov lr, lr, lsr #16
    stmia r1!, {r3-r8,r12}
    strh lr, [r1], #2
    bhi 1b
    popeq {r4-r8,pc}
2:
    add r2, r2, #0x20
3:
    ldrh r3, [r0], #2
    subs r2, r2, #2
    strh r3, [r1], #2
    bne 3b
    pop {r4-r8,pc}
    