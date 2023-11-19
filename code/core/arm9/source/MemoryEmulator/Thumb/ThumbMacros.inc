#pragma once

#define THUMB_LOAD32_TABLE_OFFSET   0x01
#define THUMB_LOAD16_TABLE_OFFSET   0x21
#define THUMB_LOAD8_TABLE_OFFSET    0x41

#define THUMB_STORE32_TABLE_OFFSET  0x61
#define THUMB_STORE16_TABLE_OFFSET  0x81
#define THUMB_STORE8_TABLE_OFFSET   0xA1

.macro memu_thumbReturn
    msr cpsr_c, #0xD7
    subs pc, lr, #6
.endm
