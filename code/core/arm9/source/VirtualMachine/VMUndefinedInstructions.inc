#pragma once

.set regToInt_r0, 0
.set regToInt_r1, 1
.set regToInt_r2, 2
.set regToInt_r3, 3
.set regToInt_r4, 4
.set regToInt_r5, 5
.set regToInt_r6, 6
.set regToInt_r7, 7
.set regToInt_r8, 8
.set regToInt_r9, 9
.set regToInt_r10, 10
.set regToInt_r11, 11
.set regToInt_r12, 12
.set regToInt_r13, 13
.set regToInt_sp, 13
.set regToInt_r14, 14
.set regToInt_lr, 14

.macro vmMRS rd, psr
    .if \psr == "cpsr"
        .word (0xE1A00090 | (regToInt_\rd))
    .elseif \psr == "spsr"
        .word (0xE1E00090 | (regToInt_\rd))
    .else
        .error "Invalid psr specified"
    .endif
.endm

.macro vmMSR psr, rd
    .if (\psr == "cpsr") || (\psr == "cpsr_fc")
        .word (0xE1890090 | (regToInt_\rd))
    .elseif \psr == "cpsr_f"
        .word (0xE1880090 | (regToInt_\rd))
    .elseif \psr == "cpsr_c"
        .word (0xE1810090 | (regToInt_\rd))
    .elseif (\psr == "spsr") || (\psr == "spsr_fc")
        .word (0xE1C90090 | (regToInt_\rd))
    .elseif \psr == "spsr_f"
        .word (0xE1C80090 | (regToInt_\rd))
    .elseif \psr == "spsr_c"
        .word (0xE1C10090 | (regToInt_\rd))
    .else
        .error "Invalid psr specified"
    .endif
.endm

.macro vmSUBS rd, rn, imm
    .if (\rd != "pc") && (\rd != "r15")
        .error "Rd must be PC"
    .else
        .word (0xEE640000 | (regToInt_\rn) | (\imm << 5))
    .endif
.endm
