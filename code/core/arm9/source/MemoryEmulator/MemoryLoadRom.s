.section ".itcm", "ax"

#include "AsmMacros.inc"
#include "VirtualMachine/VMDtcmDefs.inc"
#include "SdCache/SdCacheDefs.h"
#include "DtcmStackDefs.inc"

memu_loadRom_base:

#define DTCM(x) (memu_loadRom_base - 0x378 + (x))

arm_func memu_load32Rom
    ldr r11, DTCM(memu_adjustedRomBlockToCacheBlockAddress)
    bic r12, r8, #(3 << (SDC_BLOCK_SHIFT - 2))
arm_func memu_load32RomContinue
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
        ldrne r9, [r11, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
        bxne lr
    b memu_load32RomCacheMiss

arm_func memu_load16Rom
    ldr r11, DTCM(memu_adjustedRomBlockToCacheBlockAddress)
    bic r12, r8, #(3 << (SDC_BLOCK_SHIFT - 2))
arm_func memu_load16RomContinue
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    mov r9, r9, lsr #(32 - SDC_BLOCK_SHIFT)
    cmn r11, r8, lsl #31
        ldrgth r9, [r11, r9]
        bxgt lr

    cmp r11, #0
    ldrneh r9, [r11, r9]
    beq memu_load16RomCacheMiss
    mov r9, r9, ror #8
    bx lr

arm_func memu_load8Rom
    ldr r11, DTCM(memu_adjustedRomBlockToCacheBlockAddress)
    bic r12, r8, #(3 << (SDC_BLOCK_SHIFT - 2))
arm_func memu_load8RomContinue
    ldr r11, [r11, r12, lsr #(SDC_BLOCK_SHIFT - 2)]
    mov r9, r8, lsl #(32 - SDC_BLOCK_SHIFT)
    cmp r11, #0
        ldrneb r9, [r11, r9, lsr #(32 - SDC_BLOCK_SHIFT)]
        bxne lr
    b memu_load8RomCacheMiss
