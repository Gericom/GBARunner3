#pragma once

.macro memu_thumbReturn
    msr cpsr_c, #0xD7
    subs pc, lr, #6
.endm
