.section ".dtcm", "aw"
.altmacro

#include "AsmMacros.inc"

// .global memu_load32WordTable
// memu_load32WordTable:
    .word memu_load32Bios // 00
    .word memu_load32Undefined // 01
    .word memu_load32Ewram // 02
    .word memu_load32Iwram // 03
    .word memu_load32Io // 04
    .word memu_load32Pltt // 05
    .word memu_load32Vram012 // 06
    .word memu_load32Oam // 07
    .word memu_load32Rom // 08
    .word memu_load32Rom // 09
    .word memu_load32RomHi // 0A
    .word memu_load32RomHi // 0B
    .word memu_load32RomHi // 0C
    .word memu_load32RomHi // 0D
    .word memu_load32Sram // 0E
    .word memu_load32Sram // 0F

// .global memu_load16WordTable
// memu_load16WordTable:
    .word memu_load16Bios // 00
    .word memu_load16Undefined // 01
    .word memu_load16Ewram // 02
    .word memu_load16Iwram // 03
    .word memu_load16Io // 04
    .word memu_load16Pltt // 05
    .word memu_load16Vram012 // 06
    .word memu_load16Oam // 07
    .word memu_load16Rom // 08
    .word memu_load16Rom // 09
    .word memu_load16RomHi // 0A
    .word memu_load16RomHi // 0B
    .word memu_load16RomHi // 0C
    .word memu_load16RomHi // 0D
    .word memu_load16Sram // 0E
    .word memu_load16Sram // 0F

// .global memu_load8WordTable
// memu_load8WordTable:
    .word memu_load8Bios // 00
    .word memu_load8Undefined // 01
    .word memu_load8Ewram // 02
    .word memu_load8Iwram // 03
    .word memu_load8Io // 04
    .word memu_load8Pltt // 05
    .word memu_load8Vram012 // 06
    .word memu_load8Oam // 07
    .word memu_load8Rom // 08
    .word memu_load8Rom // 09
    .word memu_load8RomHi // 0A
    .word memu_load8RomHi // 0B
    .word memu_load8RomHi // 0C
    .word memu_load8RomHi // 0D
    .word memu_load8Sram // 0E
    .word memu_load8Sram // 0F

// .global memu_store32WordTable
// memu_store32WordTable:
    .word memu_store32Undefined // 00
    .word memu_store32Undefined // 01
    .word memu_store32Ewram // 02
    .word memu_store32Iwram // 03
    .word memu_store32Io // 04
    .word memu_store32Pltt // 05
    .word memu_store32Vram012 // 06
    .word memu_store32Oam // 07
    .word memu_store32Rom // 08
    .word memu_store32Rom // 09
    .word memu_store32Rom // 0A
    .word memu_store32Rom // 0B
    .word memu_store32Rom // 0C
    .word memu_store32Rom // 0D
    .word memu_store32Sram // 0E
    .word memu_store32Sram // 0F

// .global memu_store16WordTable
// memu_store16WordTable:
    .word memu_store16Undefined // 00
    .word memu_store16Undefined // 01
    .word memu_store16Ewram // 02
    .word memu_store16Iwram // 03
    .word memu_store16Io // 04
    .word memu_store16Pltt // 05
    .word memu_store16Vram012 // 06
    .word memu_store16Oam // 07
    .word memu_store16Rom // 08
    .word memu_store16Rom // 09
    .word memu_store16Rom // 0A
    .word memu_store16Rom // 0B
    .word memu_store16Rom // 0C
    .word memu_store16Rom // 0D
    .word memu_store16Sram // 0E
    .word memu_store16Sram // 0F

// .global memu_store8WordTable
// memu_store8WordTable:
    .word memu_store8Undefined // 00
    .word memu_store8Undefined // 01
    .word memu_store8Ewram // 02
    .word memu_store8Iwram // 03
    .word memu_store8Io // 04
    .word memu_store8Pltt // 05
    .word memu_store8Vram012 // 06
    .word memu_store8Undefined // 07, byte writes to oam are ignored
    .word memu_store8Rom // 08
    .word memu_store8Rom // 09
    .word memu_store8Rom // 0A
    .word memu_store8Rom // 0B
    .word memu_store8Rom // 0C
    .word memu_store8Rom // 0D
    .word memu_store8Sram // 0E
    .word memu_store8Sram // 0F
