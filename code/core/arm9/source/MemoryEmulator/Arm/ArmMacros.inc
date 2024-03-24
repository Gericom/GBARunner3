#pragma once

.macro memu_armReturn
    msr cpsr_c, #0xD7
    subs pc, lr, #4
.endm
