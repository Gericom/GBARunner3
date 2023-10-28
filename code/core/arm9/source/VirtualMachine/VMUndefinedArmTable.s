.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

.macro vm_armUndefinedInstructionPointer2 prefix, d, e, f, g, h, x, y
    .if (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 0)
        // MSR
        .if \h == 1
            @ .short vm_armUndefinedMsrImm\f
            .short 0
        .else
            .if \f == 1
                .short vm_armUndefinedMsrRegSpsrRmTable
            .else
                .short vm_armUndefinedMsrRegCpsrRmTable
            .endif
        .endif
    .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\g == 1) && (\h == 0)
        // MRS
        .if \f == 1
            .short vm_armUndefinedMrsSpsrRmTable
        .else
            .short vm_armUndefinedMrsCpsrRmTable
        .endif
#ifndef GBAR3_TEST
    .elseif (\prefix == 0b000) && (\d == 1) && (\e == 1) && (\f == 0) && (\g == 1) && (\h == 1)
        // BX
        .short jit_armUndefinedBxRmTable
    .elseif (\prefix == 0b110) && (\d == 0)
        // B
        .short jit_armUndefinedBTable
    .elseif (\prefix == 0b110) && (\d == 1)
        // BL
        .short jit_armUndefinedBLTable
    .elseif (\prefix == 0b111) && (\d == 0) && (\e == 1) && (\f == 0) && (\g == 0) && (\h == 0) && (\x == 0) && (\y == 1)
        // LDR pc, [Rn, +/-#imm] or LDR pc, [Rn], +/-#imm
        .short jit_armUndefinedLdrPcImmRnTable
#endif
    .elseif (\prefix == 0b111) && (\d == 0) && (\e == 0) && (\y == 0)
        // ALU pc
        .if (\f == 1) && (\g == 1)
            // ALUs pc, rn, #imm
            .short vm_armUndefinedAluSPCImmRnTable
        .else
            .short 0
        .endif
    .else
        .short 0
    .endif
.endm

.macro vm_armUndefinedInstructionPointer index
    vm_armUndefinedInstructionPointer2 %((\index>>5)&7), %((\index>>4)&1), %((\index>>3)&1), %((\index>>2)&1), %((\index>>1)&1), %((\index>>0)&1), %((\index>>8)&1), %((\index>>9)&1)
.endm

generate vm_armUndefinedInstructionPointer, 1024

.end
