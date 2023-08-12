.section ".crt0", "ax"
.arm

.global _start
.type _start, %function
_start:
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
    // setup itcm to cover the first 32KB of memory
    mov r0, #0xC
    mcr p15, 0, r0, c9, c1, 1
    // mpu region 0: ITCM, DTCM, uncached mmem, IO, GBA slot (4 GB)
    ldr r0,= ((1 | (31 << 1)) + 0x00000000)
    mcr	p15, 0, r0, c6, c0, 0
    // mpu region 1: Cached Main Memory (4 MB)
    ldr r0,= ((1 | (21 << 1)) + 0x02000000)
    mcr	p15, 0, r0, c6, c1, 0
    // mpu region 2: VRAM (8 MB)
    ldr r0,= ((1 | (22 << 1)) + 0x06000000)
    mcr	p15, 0, r0, c6, c2, 0
    // mpu region 3: Disabled
    mov r0, #0
    mcr	p15, 0, r0, c6, c3, 0
    // mpu region 4: Disabled
    mov r0, #0
    mcr	p15, 0, r0, c6, c4, 0
    // mpu region 5: LCDC VRAM A, B (256 KB)
    ldr r0,= ((1 | (17 << 1)) + 0x06800000)
    mcr	p15, 0, r0, c6, c5, 0
    // mpu region 6: IWRAM (16 MB)
    ldr r0,= (1 | (23 << 1) + 0x03000000)
    mcr	p15, 0, r0, c6, c6, 0
    // mpu region 7: GBA EWRAM (256 KB)
    ldr r0,= (1 | (17 << 1) | 0x02000000)
    mcr	p15, 0, r0, c6, c7, 0
    // data permissions
    ldr r0,= 0x33200121
	mcr p15, 0, r0, c5, c0, 2
    // code permissions
	ldr r0,= 0x33300221
	mcr p15, 0, r0, c5, c0, 3
    // dcache
	ldr r0,= 0b00100010
    mcr p15, 0, r0, c2, c0, 0
    // icache
	ldr r0,= 0b00100010
    mcr p15, 0, r0, c2, c0, 1
    // write buffer
	ldr r0,= 0b00100010
    mcr p15, 0, r0, c3, c0, 0

    // turn back on itcm, dtcm, cache and mpu
    // and use low vectors
    ldr r0,= 0x0005507D
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
    // bkpt #0

    // disable interrupts
    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0

    ldr sp,= dtcmStackEnd
    b gbaRunnerMain

.pool
.end
