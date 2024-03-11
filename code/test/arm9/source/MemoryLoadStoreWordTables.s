.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

// .global memu_load32WordTable
// memu_load32WordTable:
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

// .global memu_load16WordTable
// memu_load16WordTable:
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

// .global memu_load8WordTable
// memu_load8WordTable:
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

// .global memu_store32WordTable
// memu_store32WordTable:
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

// .global memu_store16WordTable
// memu_store16WordTable:
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

// .global memu_store8WordTable
// memu_store8WordTable:
    .word memu_store8 // 00
    .word memu_store8 // 01
    .word memu_store8 // 02
    .word memu_store8 // 03
    .word memu_store8 // 04
    .word memu_store8 // 05
    .word memu_store8 // 06
    .word memu_store8 // 07
    .word memu_store8 // 08
    .word memu_store8 // 09
    .word memu_store8 // 0A
    .word memu_store8 // 0B
    .word memu_store8 // 0C
    .word memu_store8 // 0D
    .word memu_store8 // 0E
    .word memu_store8 // 0F
