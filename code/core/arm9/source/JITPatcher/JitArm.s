.section ".itcm", "ax"
.arm

#include "AsmMacros.inc"

// r8 = address
arm_func jit_processArmBlock
arm_loop:
    ldr r12, [r8], #4
    and r13, r12, #0xE000000
    add pc, pc, r13, lsr #19
    nop
arm_000: // mrs, msr I = 0, bx, alu reg
    and r11, r12, #0xF000
    cmp r11, #0xF000
    beq arm_handleMsrRegAluRdPc
    and r10, r12, #0xF
    and r11, r12, #0xF0000
    cmp r11, #0xF0000
    beq arm_handleBxMrsAluRnPc
    cmp r10, #0xF
    bne arm_loop
    b arm_handleAluRmPc
    nop
    nop
    nop
    nop
    nop
    nop
arm_001: // msr I = 1, alu imm
    and r11, r12, #0xF000
    cmp r11, #0xF000
    beq arm_handleMsrRegAluRdPc
    and r10, r12, #0xF
    and r11, r12, #0xF0000
    cmp r11, #0xF0000
    bne arm_loop
    b arm_handleBxMrsAluRnPc
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_010: // ldr/str imm
    b arm_loop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_011: // ldr/str reg
    b arm_loop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_100: // ldm, stm
    b arm_loop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_101: // B, BL (and nothing else than that)
    eor r12, r12, #0x6000000 // 101 -> 110
    str r12, [r8, #-4]
    tst r12, #0x1000000
    beq arm_loop // bl instruction, continue scanning
    b arm_finishBlock
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_110:
    b arm_loop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
arm_111:
    b arm_loop

arm_handleBx:
    cmp r10, #0xE // bx lr
    beq arm_finishBlock // do not patch bx lr
    ldr r11,= 0x1B00090
    and r12, r12, #0xF000000F
    orr r12, r12, r11
    str r12, [r8, #-4]
    b arm_finishBlock

arm_handleMrs:
    ldr r11,= 0xF0400000
    and r11, r12, r11
    and r12, r12, #0xF000
    orr r11, r11, r12, lsr #12
    orr r11, r11, #0x90
    orr r11, r11, #0x1A00000
    str r11, [r8, #-4]
    b arm_loop

arm_handleMsr:
    ldr r11,= 0xF04F000F
    tst r12, #0x2000000
    and r11, r12, r11
    orrne r11, #0x100000
    and r12, r12, #0xFF0
    orr r11, r11, r12, lsl #4
    orr r11, r11, #0x90
    orr r11, r11, #0x1800000
    str r11, [r8, #-4]
    b arm_loop

arm_handleBxMrsAluRnPc:
    ldr r11,= 0x12FFF10
    bic r13, r12, #0xF000000F
    cmp r13, r11
    beq arm_handleBx
    and r11, r12, #0x1900000
    cmp r11, #0x1900000
    beq arm_handleMrs
arm_handleAluRnPc:
    b arm_loop

arm_handleAluRmPc:
    b arm_loop

arm_handleMsrRegAluRdPc:
    and r11, r12, #0x1900000
    cmp r11, #0x1900000
    beq arm_handleMsr
arm_handleAluRdPc:
    b arm_finishBlock

arm_finishBlock:
    bx lr
