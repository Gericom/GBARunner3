.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

.global memu_load32Table
memu_load32Table:
    .word memu_load32 // 00
    .word memu_load32 // 01
    .word memu_load32 // 02
    .word memu_load32 // 03
    .word memu_load32 // 04
    .word memu_load32 // 05
    .word memu_load32 // 06
    .word memu_load32 // 07
    .word memu_load32 // 08
    .word memu_load32 // 09
    .word memu_load32 // 0A
    .word memu_load32 // 0B
    .word memu_load32 // 0C
    .word memu_load32 // 0D
    .word memu_load32 // 0E
    .word memu_load32 // 0F

.global memu_load16Table
memu_load16Table:
    .word memu_load16 // 00
    .word memu_load16 // 01
    .word memu_load16 // 02
    .word memu_load16 // 03
    .word memu_load16 // 04
    .word memu_load16 // 05
    .word memu_load16 // 06
    .word memu_load16 // 07
    .word memu_load16 // 08
    .word memu_load16 // 09
    .word memu_load16 // 0A
    .word memu_load16 // 0B
    .word memu_load16 // 0C
    .word memu_load16 // 0D
    .word memu_load16 // 0E
    .word memu_load16 // 0F

.global memu_load8Table
memu_load8Table:
    .word memu_load8 // 00
    .word memu_load8 // 01
    .word memu_load8 // 02
    .word memu_load8 // 03
    .word memu_load8 // 04
    .word memu_load8 // 05
    .word memu_load8 // 06
    .word memu_load8 // 07
    .word memu_load8 // 08
    .word memu_load8 // 09
    .word memu_load8 // 0A
    .word memu_load8 // 0B
    .word memu_load8 // 0C
    .word memu_load8 // 0D
    .word memu_load8 // 0E
    .word memu_load8 // 0F

.global memu_store32Table
memu_store32Table:
    .word memu_store32 // 00
    .word memu_store32 // 01
    .word memu_store32 // 02
    .word memu_store32 // 03
    .word memu_store32 // 04
    .word memu_store32 // 05
    .word memu_store32 // 06
    .word memu_store32 // 07
    .word memu_store32 // 08
    .word memu_store32 // 09
    .word memu_store32 // 0A
    .word memu_store32 // 0B
    .word memu_store32 // 0C
    .word memu_store32 // 0D
    .word memu_store32 // 0E
    .word memu_store32 // 0F

.global memu_store16Table
memu_store16Table:
    .word memu_store16 // 00
    .word memu_store16 // 01
    .word memu_store16 // 02
    .word memu_store16 // 03
    .word memu_store16 // 04
    .word memu_store16 // 05
    .word memu_store16 // 06
    .word memu_store16 // 07
    .word memu_store16 // 08
    .word memu_store16 // 09
    .word memu_store16 // 0A
    .word memu_store16 // 0B
    .word memu_store16 // 0C
    .word memu_store16 // 0D
    .word memu_store16 // 0E
    .word memu_store16 // 0F

.global memu_store8Table
memu_store8Table:
    .word memu_store8 // 00
    .word memu_store8 // 01
    .word memu_store8 // 02
    .word memu_store8 // 03
    .word memu_store8 // 04
    .word memu_store8 // 05
    .word memu_store8 // 06
    .word memu_store8 // 07, byte writes to oam are ignored
    .word memu_store8 // 08
    .word memu_store8 // 09
    .word memu_store8 // 0A
    .word memu_store8 // 0B
    .word memu_store8 // 0C
    .word memu_store8 // 0D
    .word memu_store8 // 0E
    .word memu_store8 // 0F
