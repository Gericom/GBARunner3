.text

.global _dldi_start
_dldi_start:
    .space 0x60

.global _io_dldi
_io_dldi:
    .space 8
.global _DLDI_startup_ptr
_DLDI_startup_ptr:
    .word 0
.global _DLDI_isInserted_ptr
_DLDI_isInserted_ptr:
    .word 0
.global _DLDI_readSectors_ptr
_DLDI_readSectors_ptr:
    .word 0
.global _DLDI_writeSectors_ptr
_DLDI_writeSectors_ptr:
    .word 0
.global _DLDI_clearStatus_ptr
_DLDI_clearStatus_ptr:
    .word 0
.global _DLDI_shutdown_ptr
_DLDI_shutdown_ptr:
    .word 0

dldi_data_end:

.space 16384 - (dldi_data_end - _dldi_start)    // 16 kB

_dldi_end:

.end
