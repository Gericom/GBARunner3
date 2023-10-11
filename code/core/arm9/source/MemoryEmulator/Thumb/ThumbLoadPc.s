.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"

#ifdef GBAR3_HICODE_CACHE_MAPPING

.macro memu_thumbLdrPcRd rd
    arm_func memu_thumbLdrPcR\rd
        ldr r10,= memu_inst_addr
        and r8, r11, #0xFF
        ldr r10, [r10]
        sub r8, r8, #1
        bic r10, r10, #3
        add r8, r10, r8, lsl #2

        and r10, r8, #0x0F000000
        cmp r8, #0x10000000
            addlo r9, r9, r10, lsr #22
        ldr r10, [r9, #1] // memu_load32Table
        // interlock
        blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrPcRd, 8

#endif
