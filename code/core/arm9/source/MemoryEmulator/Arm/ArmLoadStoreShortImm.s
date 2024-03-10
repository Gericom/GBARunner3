.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func memu_armLoadStoreShortImm0_0
    and r12, lr, #0xF00
    sub r9, r9, r12, lsr #4 // u=0 -> negate offset
    bx r8

arm_func memu_armLoadStoreShortImm_0
    mov r9, r9, lsr #2
    and r12, lr, #0xF00
    orr r9, r9, r12, lsr #4
    rsb r9, r9, #0 // u=0 -> negate offset
    bx r8

arm_func memu_armLoadStoreShortImm0_1
    and r12, lr, #0xF00
    mov r9, r12, lsr #4
    bx r8

arm_func memu_armLoadStoreShortImm_1
    mov r9, r9, lsr #2
    and r12, lr, #0xF00
    orr r9, r9, r12, lsr #4
    bx r8

.section ".dtcm", "aw"

.balign 64

.global memu_armLoadStoreShortImmTable_0
memu_armLoadStoreShortImmTable_0:
    .word memu_armLoadStoreShortImm0_0
    .rept 15
        .word memu_armLoadStoreShortImm_0
    .endr

 .global memu_armLoadStoreShortImmTable_1
memu_armLoadStoreShortImmTable_1:
    .word memu_armLoadStoreShortImm0_1
    .rept 15
        .word memu_armLoadStoreShortImm_1
    .endr

.end
