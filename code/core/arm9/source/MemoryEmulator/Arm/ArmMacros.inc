#pragma once

#define ARM_LOAD32_TABLE_OFFSET     (-0xC4)
#define ARM_LOAD16_TABLE_OFFSET     (-0xA4)
#define ARM_LOAD8_TABLE_OFFSET      (-0x84)

#define ARM_STORE32_TABLE_OFFSET    (-0x64)
#define ARM_STORE16_TABLE_OFFSET    (-0x44)
#define ARM_STORE8_TABLE_OFFSET     (-0x24)

.macro memu_armReturn
    msr cpsr_c, #0xD7
    subs pc, lr, #4
.endm
