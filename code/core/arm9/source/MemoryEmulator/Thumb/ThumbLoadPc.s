.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"
#include "ThumbMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "SdCache/SdCacheDefs.h"

#ifdef GBAR3_HICODE_CACHE_MAPPING

.macro memu_thumbLdrPcRd rd
    .balign 8
    arm_func memu_thumbLdrPcR\rd
        ldr r10,= memu_inst_addr
        and r8, r11, #0xFF
        ldr r10, [r10]
        sub r8, r8, #1
        bic r10, r10, #3
        add r8, r10, r8, lsl #2

        bl memu_load32Rom

        @ add r9, r9, r8, lsr #23
        @ ldrh r10, [r9, #THUMB_LOAD32_TABLE_OFFSET] // memu_load32Table
        @ cmp r8, #0x10000000
        @     ldrhs r10,= memu_load32Undefined
        @ blx r10

        mov r\rd, r9
        memu_thumbReturn
.endm

generate memu_thumbLdrPcRd, 8

#endif
