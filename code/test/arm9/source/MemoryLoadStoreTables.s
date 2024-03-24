.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

// .global memu_load32Table
// memu_load32Table:
    .short memu_load32 // 00
    .short memu_load32 // 01
    .short memu_load32 // 02
    .short memu_load32 // 03
    .short memu_load32 // 04
    .short memu_load32 // 05
    .short memu_load32 // 06
    .short memu_load32 // 07
    .short memu_load32 // 08
    .short memu_load32 // 09
    .short memu_load32 // 0A
    .short memu_load32 // 0B
    .short memu_load32 // 0C
    .short memu_load32 // 0D
    .short memu_load32 // 0E
    .short memu_load32 // 0F

// .global memu_load16Table
// memu_load16Table:
    .short memu_load16 // 00
    .short memu_load16 // 01
    .short memu_load16 // 02
    .short memu_load16 // 03
    .short memu_load16 // 04
    .short memu_load16 // 05
    .short memu_load16 // 06
    .short memu_load16 // 07
    .short memu_load16 // 08
    .short memu_load16 // 09
    .short memu_load16 // 0A
    .short memu_load16 // 0B
    .short memu_load16 // 0C
    .short memu_load16 // 0D
    .short memu_load16 // 0E
    .short memu_load16 // 0F

// .global memu_load8Table
// memu_load8Table:
    .short memu_load8 // 00
    .short memu_load8 // 01
    .short memu_load8 // 02
    .short memu_load8 // 03
    .short memu_load8 // 04
    .short memu_load8 // 05
    .short memu_load8 // 06
    .short memu_load8 // 07
    .short memu_load8 // 08
    .short memu_load8 // 09
    .short memu_load8 // 0A
    .short memu_load8 // 0B
    .short memu_load8 // 0C
    .short memu_load8 // 0D
    .short memu_load8 // 0E
    .short memu_load8 // 0F

// .global memu_store32Table
// memu_store32Table:
    .short memu_store32 // 00
    .short memu_store32 // 01
    .short memu_store32 // 02
    .short memu_store32 // 03
    .short memu_store32 // 04
    .short memu_store32 // 05
    .short memu_store32 // 06
    .short memu_store32 // 07
    .short memu_store32 // 08
    .short memu_store32 // 09
    .short memu_store32 // 0A
    .short memu_store32 // 0B
    .short memu_store32 // 0C
    .short memu_store32 // 0D
    .short memu_store32 // 0E
    .short memu_store32 // 0F

// .global memu_store16Table
// memu_store16Table:
    .short memu_store16 // 00
    .short memu_store16 // 01
    .short memu_store16 // 02
    .short memu_store16 // 03
    .short memu_store16 // 04
    .short memu_store16 // 05
    .short memu_store16 // 06
    .short memu_store16 // 07
    .short memu_store16 // 08
    .short memu_store16 // 09
    .short memu_store16 // 0A
    .short memu_store16 // 0B
    .short memu_store16 // 0C
    .short memu_store16 // 0D
    .short memu_store16 // 0E
    .short memu_store16 // 0F

// .global memu_store8Table
// memu_store8Table:
    .short memu_store8 // 00
    .short memu_store8 // 01
    .short memu_store8 // 02
    .short memu_store8 // 03
    .short memu_store8 // 04
    .short memu_store8 // 05
    .short memu_store8 // 06
    .short memu_store8 // 07
    .short memu_store8 // 08
    .short memu_store8 // 09
    .short memu_store8 // 0A
    .short memu_store8 // 0B
    .short memu_store8 // 0C
    .short memu_store8 // 0D
    .short memu_store8 // 0E
    .short memu_store8 // 0F
