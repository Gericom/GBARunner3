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

.global memu_stubStoredAddresses32
memu_stubStoredAddresses32:
    .space (4 * 16)
.global memu_stubStoredValues32
memu_stubStoredValues32:
    .space (4 * 16)
.global memu_stubStore32Count
memu_stubStore32Count:
    .word 0

arm_func memu_store32
    adr r10, memu_stubStoredAddresses32
    ldr r11, memu_stubStore32Count
    str r8, [r10, r11, lsl #2]
    adr r10, memu_stubStoredValues32
    str r9, [r10, r11, lsl #2]
    add r11, r11, #1
    str r11, memu_stubStore32Count
    bx lr

.global memu_stubLoadedAddress8
memu_stubLoadedAddress8:
    .word 0
.global memu_stubLoadValue8
memu_stubLoadValue8:
    .word 0

arm_func memu_load8
arm_func memu_load8Undefined
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
arm_func memu_load16Undefined
    str r8, memu_stubLoadedAddress16
    ldr r9, memu_stubLoadValue16
    bx lr

.global memu_stubLoadedAddresses32
memu_stubLoadedAddresses32:
    .space (4 * 16)
.global memu_stubLoadValues32
memu_stubLoadValues32:
    .space (4 * 16)
.global memu_stubLoad32Count
memu_stubLoad32Count:
    .word 0

arm_func memu_load32
arm_func memu_load32Undefined
    adr r10, memu_stubLoadedAddresses32
    ldr r11, memu_stubLoad32Count
    str r8, [r10, r11, lsl #2]
    adr r10, memu_stubLoadValues32
    ldr r9, [r10, r11, lsl #2]
    add r11, r11, #1
    str r11, memu_stubLoad32Count
    bx lr
