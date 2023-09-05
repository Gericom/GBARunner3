.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

arm_func jit_thumbBx
    and r12, r0, #(0xF << 3)
    ldr r12, [sp, r12, lsr #1]
    tst r12, #1
        biceq r12, r12, #2
    sub lr, r12, #0x02200000
    cmp lr, #0x00200000
    bhs vm_undefinedThumbContinue

ensureJittedStaticRom:
    ldr r11,= gStaticRomJitBits
    movs lr, lr, lsr #1
        biccc r10, r10, #0x20 // thumb bit
    ldrb r11, [r11, lr, lsr #3]
    and lr, lr, #0x7
    rsb lr, lr, #32
    movs r11, r11, lsl lr
    bcc vm_undefinedThumbContinue
    ldmia sp, {r0-r1}
    msr spsr, r10
    movs pc, r12
