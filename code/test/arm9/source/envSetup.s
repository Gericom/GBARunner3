.text
.arm

.global main
.type main, %function
main:
    // disable irqs
    msr cpsr, #0x9F
    
    // configure cp15
    // disable itcm, dtcm, caches and mpu
    ldr r0,= 0x00002078
    mcr p15, 0, r0, c1, c0
    mov r0, #0
    // drain write buffer
    mcr p15, 0, r0, c7, c10, 4
    // invalidate entire icache
    mcr p15, 0, r0, c7, c5, 0
    @ // invalidate entire dcache
    @ mcr p15, 0, r0, c7, c6, 0
    // move dtcm in place
    ldr r0,= 0x8000002C
    mcr p15, 0, r0, c9, c1, 0
    // setup itcm to cover the first 32KB of memory
    mov r0, #0xC
    mcr p15, 0, r0, c9, c1, 1
    // mpu region 0: IO, Palette, VRAM, OAM (64 MB)
    ldr r0,= ((1 | (25 << 1)) + 0x04000000)
    mcr	p15, 0, r0, c6, c0, 0
    // mpu region 1: Main Memory + TWL WRAM (32 MB)
    ldr r0,= ((1 | (24 << 1)) + 0x02000000)
    mcr	p15, 0, r0, c6, c1, 0
    // mpu region 2: GBA slot
    ldr r0,= ((1 | (24 << 1)) + 0x08000000)
    mcr	p15, 0, r0, c6, c2, 0
    // mpu region 3: Disabled
    mov r0, #0
    mcr	p15, 0, r0, c6, c3, 0
    // mpu region 4: Disabled
    mov r0, #0
    mcr	p15, 0, r0, c6, c4, 0
    // mpu region 5: ITCM (32 KB)
    ldr r0,= (1 | (14 << 1))
    mcr	p15, 0, r0, c6, c5, 0
    // mpu region 6: DTCM (2 GB)
    ldr r0,= (1 | (30 << 1) + 0x80000000)
    mcr	p15, 0, r0, c6, c6, 0
    // mpu region 7: LCDC VRAM A+B (256 KB)
    ldr r0,= (1 | (17 << 1) | 0x06800000)
    mcr	p15, 0, r0, c6, c7, 0
    // data permissions
    ldr r0,= 0x33300333
	mcr p15, 0, r0, c5, c0, 2
    // code permissions
	ldr r0,= 0x30300330
	mcr p15, 0, r0, c5, c0, 3
    // dcache
	ldr r0,= 0b10000010
    mcr p15, 0, r0, c2, c0, 0
    // icache
	ldr r0,= 0b10000010
    mcr p15, 0, r0, c2, c0, 1
    // write buffer
	ldr r0,= 0b10000010
    mcr p15, 0, r0, c3, c0, 0

    // turn back on itcm, dtcm, cache and mpu
    // and use low vectors
    ldr r0,= 0x0005507D
    mcr p15, 0, r0, c1, c0
    
    // switch to user mode
    msr cpsr, #0x90

    ldr sp,= __dtcm_start + (8 * 1024)
    b testMain