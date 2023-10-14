.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

.macro vm_thumbUndefinedInstructionPointer2 index, rm, cond
#ifndef GBAR3_TEST
    .if ((\index >> 4) == 0b10001) // bx Rm
        .short jit_thumbUndefinedBxR\rm
    .elseif ((\index >> 4) == 0b11110) // mov pc, Rm
        .short jit_thumbUndefinedMovPcR\rm
    .elseif ((\index >> 4) == 0b11111) // add pc, Rm
        .short jit_thumbUndefinedAddPcR\rm
    .elseif ((\index >> 3) == 0b110111) // pop {.., pc}
        .short vm_undefinedThumbNotInTable
    .elseif ((\index >> 6) == 0b110) // b_cond imm
        .short jit_thumbUndefinedBCond\cond
    .elseif (((\index >> 7) == 0b10) && ((\index >> 5) == 1)) // b imm
        .short vm_undefinedThumbNotInTable
    .elseif ((\index >> 7) == 0b01) // bl lr+imm
        .short vm_undefinedThumbNotInTable
    .else
        .short vm_undefinedThumbNotInTable
    .endif
#else
    .short vm_undefinedThumbNotInTable
#endif
.endm

.macro vm_thumbUndefinedInstructionPointer index
    vm_thumbUndefinedInstructionPointer2 \index, %((\index) & 0xF), %(((\index) >> 2) & 0xF)
.endm

generate vm_thumbUndefinedInstructionPointer, 512

.end
