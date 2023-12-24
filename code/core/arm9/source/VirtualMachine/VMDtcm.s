.section ".dtcm", "aw"

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"
#include "SdCache/SdCacheDefs.h"

.org vm_hwIEAddr - VM_DTCM_BASE
    .word 0x04000210

.org vm_emulatedIrqMask - VM_DTCM_BASE
    .word 0xFFFFEF80 // keys, timers and display irqs have a hw source

.org vm_undefinedArmTableAddr - VM_DTCM_BASE
    .word vm_armUndefinedDispatchTable

.org vm_undefinedThumbTableAddr - VM_DTCM_BASE
    .word vm_thumbUndefinedDispatchTable

.org vm_svcRegsAddr - VM_DTCM_BASE
    .word vm_regs_svc

.org vm_modeSwitchTableAddr - VM_DTCM_BASE
    .word vm_modeSwitchTable

.org memu_inst_addr - VM_DTCM_BASE
    .word 0

.org memu_thumb_table_addr - VM_DTCM_BASE
    .word 0x80000000

.org memu_arm_table_addr - VM_DTCM_BASE
    .word memu_armDispatchTable

.org dma_stepTable - VM_DTCM_BASE
    .byte 1, -1, 0, 1

.org memu_biosOpcodeId - VM_DTCM_BASE
    .word MEMU_BIOS_OPCODE_ID_RESET

.org memu_adjustedRomBlockToCacheBlockAddress - VM_DTCM_BASE
#ifndef GBAR3_TEST
    .word (sdc_romBlockToCacheBlock - (0x08000000 >> (SDC_BLOCK_SHIFT - 2)))
#else
    .word 0
#endif

.end
