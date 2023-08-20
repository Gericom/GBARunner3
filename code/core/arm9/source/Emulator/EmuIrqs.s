.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

/// @brief Updates DS REG_IE and triggers a GX fifo irq when there are
///        any pending emulated irqs, such that the irq handler will
///        automatically be called again as soon as irqs are turned back on.
/// @param r0-r9 Preserved
/// @param r10-r12 Trashed
/// @param r13 Preserved
/// @param lr Return address.
arm_func emu_updateIrqs
    ldr r12,= vm_forcedIrqMask
    ldr r11, [r12, #(vm_cpsr - vm_forcedIrqMask)]
    ldr r10, [r12]
    tst r11, #0x80 // cpsr I flag
    ldr r11, [r12, #(vm_emulatedIrqMask - vm_forcedIrqMask)]
    ldreq r12, [r12, #(vm_emulatedIfImeIe - vm_forcedIrqMask)]
    movne r12, #0
    tst r12, #0x8000
        moveq r12, #0
    tst r12, r12, lsl #17 // IF & IE
    bic r12, r12, r11, lsl #17
    orr r10, r10, r12, lsr #17
    mov r11, #0x04000000
    str r10, [r11, #0x210] // store IE mask in REG_IE
    bxeq lr // return directly if there are no pending irqs to be handled

    mov r12, #0x80000000
    str r12, [r11, #0x600] // enabled GX fifo irq when empty (=always true)
    mov r12, #0
    str r12, [r11, #0x600] // disable GX fifo irq
    bx lr
    