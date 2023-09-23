.section ".dtcm", "aw"

#include "AsmMacros.inc"
#include "VMDtcmDefs.inc"

.org vm_toIrqSwitchTableAddr - VM_DTCM_BASE
    .word (vm_modeSwitchTable + (2 << 4) * 4 - 0xFFFFC000 + 0x80000000)

.org vm_hwIEAddr - VM_DTCM_BASE
    .word 0x04000210

.org vm_emulatedIrqMask - VM_DTCM_BASE
    .word 0xFFFFFF80 // timers and display irqs have a hw source

.org vm_undefinedArmTableAddr - VM_DTCM_BASE
    .word vm_armUndefinedDispatchTable

.org vm_svcRegsAddr - VM_DTCM_BASE
    .word vm_regs_svc

.org vm_modeSwitchTableAddr - VM_DTCM_BASE
    .word vm_modeSwitchTable

.end
