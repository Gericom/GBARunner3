.text
.altmacro

#include "AsmMacros.inc"

arm_func sav_swiHandler
    adr r12, (sav_swiTable - (0x80 * 4))
    ldr r12, [r12, r13, lsl #2]
    msr cpsr_c, #0x1F
    push {lr}
    blx r12
    pop {r12}
    msr cpsr_c, #0x93
    tst r12, #1
    msreq spsr, #0x10
    msrne spsr, #0x30
    bics pc, r12, #1

.global sav_swiTable
sav_swiTable:
    .space 16 * 4

thumb_func sav_callSwi0
    swi 0x80
thumb_func sav_callSwi1
    swi 0x81
thumb_func sav_callSwi2
    swi 0x82
thumb_func sav_callSwi3
    swi 0x83
thumb_func sav_callSwi4
    swi 0x84
thumb_func sav_callSwi5
    swi 0x85
thumb_func sav_callSwi6
    swi 0x86
thumb_func sav_callSwi7
    swi 0x87
thumb_func sav_callSwi8
    swi 0x88
thumb_func sav_callSwi9
    swi 0x89
thumb_func sav_callSwi10
    swi 0x8A
thumb_func sav_callSwi11
    swi 0x8B
thumb_func sav_callSwi12
    swi 0x8C
thumb_func sav_callSwi13
    swi 0x8D
thumb_func sav_callSwi14
    swi 0x8E
thumb_func sav_callSwi15
    swi 0x8F
