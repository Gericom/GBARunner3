.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func memu_armLoadStoreImm_0
    and r9, lr, r13, lsr #20 // r9 = offset_12
    rsb r9, r9, #0 // u=0 -> negate offset
    bx r8

arm_func memu_armLoadStoreImm_1
    and r9, lr, r13, lsr #20 // r9 = offset_12
    bx r8

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadStoreImmTable_0
memu_armLoadStoreImmTable_0:
    .rept 16
        .word memu_armLoadStoreImm_0
    .endr

 .global memu_armLoadStoreImmTable_1
memu_armLoadStoreImmTable_1:
    .rept 16
        .word memu_armLoadStoreImm_1
    .endr   

.end
