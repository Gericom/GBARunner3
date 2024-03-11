.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "MemoryEmulator/RomDefs.h"

.macro jit_armUndefinedLdrPcImmRn rn
    arm_func jit_armUndefinedLdrPcImmR\rn
        // todo: we assume pre increment and no writeback for now
        ldr r9, [r13, #(vm_undefinedRegTmp - vm_armUndefinedDispatchTable)]! // dummy read, only used to compute an address
        .if \rn < 8
            mov r8, r\rn
        .elseif \rn < 15
            stmia r13, {r\rn}^
            nop
            ldr r8, [r13]
        .else
            // pc
            add r8, r11, #4
            cmp r11, #ROM_LINEAR_DS_ADDRESS
            blo 1f
            cmp r11, #ROM_LINEAR_END_DS_ADDRESS
            bhs 1f
            add r8, r8, #(ROM_LINEAR_GBA_ADDRESS - ROM_LINEAR_DS_ADDRESS)
        1:
        .endif
        b jit_armUndefinedLdrPcImmCommon
.endm

generate jit_armUndefinedLdrPcImmRn, 16

jit_armUndefinedLdrPcImmCommon:
    tst lr, #0x200
    and r9, lr, #0x000001E0
    and lr, lr, #0x000FF000
    orr r9, r9, lr, lsr #3
    addne r8, r8, r9, lsr #5
    subeq r8, r8, r9, lsr #5
    bic r8, r8, #3
    bl memu_load32
    ldr r10, [r13, #(vm_undefinedSpsr - vm_undefinedRegTmp)]
    ldr sp,= dtcmStackEnd
    bic r9, r9, #3
    push {r0-r3}
    mov r0, r9
    bl jit_ensureBlockJitted
    pop {r0-r3}
    msr spsr, r10
    movs pc, r9

.section ".dtcm", "aw"

.balign 64

.global jit_armUndefinedLdrPcImmRnTable
jit_armUndefinedLdrPcImmRnTable:
    .word jit_armUndefinedLdrPcImmR0
    .word jit_armUndefinedLdrPcImmR1
    .word jit_armUndefinedLdrPcImmR2
    .word jit_armUndefinedLdrPcImmR3
    .word jit_armUndefinedLdrPcImmR4
    .word jit_armUndefinedLdrPcImmR5
    .word jit_armUndefinedLdrPcImmR6
    .word jit_armUndefinedLdrPcImmR7
    .word jit_armUndefinedLdrPcImmR8
    .word jit_armUndefinedLdrPcImmR9
    .word jit_armUndefinedLdrPcImmR10
    .word jit_armUndefinedLdrPcImmR11
    .word jit_armUndefinedLdrPcImmR12
    .word jit_armUndefinedLdrPcImmR13
    .word jit_armUndefinedLdrPcImmR14
    .word jit_armUndefinedLdrPcImmR15
