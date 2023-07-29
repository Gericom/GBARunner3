.section ".dtcm", "aw"

.global memu_inst_addr
memu_inst_addr:
    .word 0

.space 0xC

.global memu_thumb_table_addr
memu_thumb_table_addr:
    .word 0x800000FC

.global memu_arm_table_addr
memu_arm_table_addr:
    .word memu_armDispatchTable

.end