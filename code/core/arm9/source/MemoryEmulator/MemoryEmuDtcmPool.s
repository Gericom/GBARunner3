.section ".dtcm", "aw"

#include "MemoryEmuDtcm.inc"

.org memu_inst_addr - MEMU_DTCM_BASE
    .word 0

.org memu_thumb_table_addr - MEMU_DTCM_BASE
    .word 0x80000000

.org memu_arm_table_addr - MEMU_DTCM_BASE
    .word memu_armDispatchTable

.end