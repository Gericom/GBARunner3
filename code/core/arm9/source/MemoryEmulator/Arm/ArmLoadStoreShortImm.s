.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func memu_armLoadStoreShortImm_0
    and r12, lr, #0xF00
    orr r9, r9, r12, lsr #4
    rsb r9, r9, #0 // u=0 -> negate offset
    bx r8

arm_func memu_armLoadStoreShortImm_1
    and r12, lr, #0xF00
    orr r9, r9, r12, lsr #4
    bx r8

.section ".dtcm", "aw"

.global memu_armLoadStoreShortImmTable_0
memu_armLoadStoreShortImmTable_0:
    .rept 16
        .word memu_armLoadStoreShortImm_0
    .endr

 .global memu_armLoadStoreShortImmTable_1
memu_armLoadStoreShortImmTable_1:
    .rept 16
        .word memu_armLoadStoreShortImm_1
    .endr   

.end
