.section ".crt0", "ax"
.arm

.global _start
.type _start, %function
_start:
    mov r11, r0 // argc
    mov r12, r1 // argv
    // configure cp15
    // disable itcm, dtcm, caches and mpu
    ldr r0,= 0x00002078
    mcr p15, 0, r0, c1, c0
    mov r0, #0
    // drain write buffer
    mcr p15, 0, r0, c7, c10, 4
    // invalidate entire icache
    mcr p15, 0, r0, c7, c5, 0
    // invalidate entire dcache
    @ mcr p15, 0, r0, c7, c6, 0
    // move dtcm in place
    ldr r0,= 0x8000002C
    mcr p15, 0, r0, c9, c1, 0
    // setup itcm to cover the first 32MB of memory
    mov r0, #0x20
    mcr p15, 0, r0, c9, c1, 1

    // iwram to arm 9
    ldr r0,= 0x04000247
    mov r1, #0
    strb r1, [r0]

    ldr sp,= 0x03008000
    push {r12}
    bl setupMemoryProtection
    pop {r12}

    // turn back on itcm, dtcm, cache and mpu
    // and use low vectors and armv4t backwards compatibility
    ldr r0,= 0x0005D07D
    mcr p15, 0, r0, c1, c0

    // copy itcm in place
    ldr r0,= __itcm_lma
    ldr r2,= __itcm_start
    ldr r1,= __itcm_end
    subs r1, r1, r2
    beq itcm_done
1:
    ldmia r0!, {r3-r10}
    stmia r2!, {r3-r10}
    subs r1, #0x20
    bgt 1b
itcm_done:

    // copy dtcm in place
    ldr r0,= __dtcm_lma
    ldr r2,= __dtcm_start
    ldr r1,= __dtcm_end
    subs r1, r1, r2
    beq dtcm_done
1:
    ldmia r0!, {r3-r10}
    stmia r2!, {r3-r10}
    subs r1, #0x20
    bgt 1b
dtcm_done:

    // clear bss
    ldr r0,= __bss_start
    ldr r1,= __bss_end
    cmp r0, r1
    beq bss_done
    mov r2, #0
1:
    str r2, [r0], #4
    cmp r0, r1
    bne 1b
bss_done:

    // clear ewram bss
    ldr r0,= __ewram_bss_start
    ldr r1,= __ewram_bss_end
    cmp r0, r1
    beq ewram_bss_done
    mov r2, #0
1:
    str r2, [r0], #4
    cmp r0, r1
    bne 1b
ewram_bss_done:

    // clear vram hi bss
    ldr r0,= __vramhi_bss_start
    ldr r1,= __vramhi_bss_end
    cmp r0, r1
    beq vramhi_bss_done
    mov r2, #0
1:
    str r2, [r0], #4
    cmp r0, r1
    bne 1b
vramhi_bss_done:
    // bkpt #0

    // disable interrupts
    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0

    ldr sp,= dtcmStackEnd

    push {r11, r12}
    bl __libc_init_array
    pop {r0, r1}

    // idk why this is needed
    nop

    b gbaRunnerMain

.pool
.end
