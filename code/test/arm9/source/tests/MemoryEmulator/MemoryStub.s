.section ".itcm", "ax"

#include "AsmMacros.inc"

.global memu_stubStoredAddress8
memu_stubStoredAddress8:
    .word 0
.global memu_stubStoredValue8
memu_stubStoredValue8:
    .word 0

arm_func memu_store8
    str r8, memu_stubStoredAddress8
    str r9, memu_stubStoredValue8
    bx lr

.global memu_stubStoredAddress16
memu_stubStoredAddress16:
    .word 0
.global memu_stubStoredValue16
memu_stubStoredValue16:
    .word 0

arm_func memu_store16
    str r8, memu_stubStoredAddress16
    str r9, memu_stubStoredValue16
    bx lr

.global memu_stubStoredAddress32
memu_stubStoredAddress32:
    .word 0
.global memu_stubStoredValue32
memu_stubStoredValue32:
    .word 0

arm_func memu_store32
    str r8, memu_stubStoredAddress32
    str r9, memu_stubStoredValue32
    bx lr

.global memu_stubLoadedAddress8
memu_stubLoadedAddress8:
    .word 0
.global memu_stubLoadValue8
memu_stubLoadValue8:
    .word 0

arm_func memu_load8
    str r8, memu_stubLoadedAddress8
    ldr r9, memu_stubLoadValue8
    bx lr

.global memu_stubLoadedAddress16
memu_stubLoadedAddress16:
    .word 0
.global memu_stubLoadValue16
memu_stubLoadValue16:
    .word 0

arm_func memu_load16
    str r8, memu_stubLoadedAddress16
    ldr r9, memu_stubLoadValue16
    bx lr

.global memu_stubLoadedAddress32
memu_stubLoadedAddress32:
    .word 0
.global memu_stubLoadValue32
memu_stubLoadValue32:
    .word 0

arm_func memu_load32
    str r8, memu_stubLoadedAddress32
    ldr r9, memu_stubLoadValue32
    bx lr
