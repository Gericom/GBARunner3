#pragma once

.global memu_load32WordTable
.set memu_load32WordTable, 0xFFFFF4E8

.global memu_load16WordTable
.set memu_load16WordTable, 0xFFFFF528

.global memu_load8WordTable
.set memu_load8WordTable, 0xFFFFF568

.global memu_store32WordTable
.set memu_store32WordTable, 0xFFFFF5A8

.global memu_store16WordTable
.set memu_store16WordTable, 0xFFFFF5E8

.global memu_store8WordTable
.set memu_store8WordTable, 0xFFFFF628

.global memu_load32Table
.set memu_load32Table, 0xFFFFFF40

.global memu_load16Table
.set memu_load16Table, 0xFFFFFF60

.global memu_load8Table
.set memu_load8Table, 0xFFFFFF80

.global memu_store32Table
.set memu_store32Table, 0xFFFFFFA0

.global memu_store16Table
.set memu_store16Table, 0xFFFFFFC0

.global memu_store8Table
.set memu_store8Table, 0xFFFFFFE0
