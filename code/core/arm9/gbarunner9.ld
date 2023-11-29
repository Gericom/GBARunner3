/*--------------------------------------------------------------------------------
	This Source Code Form is subject to the terms of the Mozilla Public License,
	v. 2.0. If a copy of the MPL was not distributed with this file, You can
	obtain one at https://mozilla.org/MPL/2.0/.
--------------------------------------------------------------------------------*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	itcm	: ORIGIN = 0x00000000, LENGTH = 32K
	ewram   : ORIGIN = 0x02040000, LENGTH = 4M - 2M - 256K
	dtcm	: ORIGIN = 0xFFFFC000, LENGTH = 16K
	vrama	: ORIGIN = 0x06800000, LENGTH = 128K
	vramab	: ORIGIN = 0x06800000, LENGTH = 256K
	vramhi  : ORIGIN = 0x06898000, LENGTH = 48K
}

PHDRS
{
	main    PT_LOAD FLAGS(7);
	itcm    PT_LOAD FLAGS(7);
	dtcm    PT_LOAD FLAGS(7);
}

__heap_end = ORIGIN(ewram) + LENGTH(ewram);

SECTIONS
{
	.crt0	:
	{
		__text_start = . ;
		KEEP (*(.crt0))
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.text :   /* ALIGN (4): */
	{
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .text)
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .stub)
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .text.*)
		KEEP (*(SORT_NONE(.init)))
		*(.text.*)
		*(.stub)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		__glue_start = ABSOLUTE(.);
		*(.glue_7)
		*(.glue_7t)
		__glue_end = ABSOLUTE(.);
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.fini           :
	{
		KEEP (*(.fini))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main =0xff

	__text_end = . ;

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >vrama AT> vramab :main
	__exidx_start = .;
	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >vrama AT> vramab :main
	__exidx_end = .;

/* Ensure the __preinit_array_start label is properly aligned.  We
   could instead move the label definition inside the section, but
   the linker would then create the section even if it turns out to
   be empty, which isn't pretty.  */
	. = ALIGN(32 / 8);
	PROVIDE (__preinit_array_start = .);
	.preinit_array     : { KEEP (*(.preinit_array)) } >vrama AT> vramab :main = 0xff
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array     : { KEEP (*(.init_array)) } >vrama AT> vramab :main = 0xff
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     : { KEEP (*(.fini_array)) } >vrama AT> vramab :main = 0xff
	PROVIDE (__fini_array_end = .);

	.ctors :
	{
	/* gcc uses crtbegin.o to find the start of the constructors, so
		we make sure it is first.  Because this is a wildcard, it
		doesn't matter if the user does not actually link against
		crtbegin.o; the linker won't look for a file to match a
		wildcard.  The wildcard also means that it doesn't matter which
		directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >vrama AT> vramab :main = 0xff
	.jcr            : { KEEP (*(.jcr)) } >vrama AT> vramab = 0


	__got_start = . ;
	.got            : 
	{ 
		*(.got.plt) 
		*(.got) 
		*(.rel.got)
	} >vrama AT> vramab :main = 0
	__got_end = . ;

	
	.data ALIGN(4) : 	{
		__data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >vrama AT> vramab :main = 0xff

	__data_end = . ;

	__bss_vma = . ;

	.dtcm :
	{
		__dtcm_lma = LOADADDR(.dtcm);
		__dtcm_start = ABSOLUTE(.);
		KEEP (ThumbDtcm.o(.dtcm))
		. = 0x1000;
		KEEP (MemoryLoadStoreTables.o(.dtcm))
		. = 0x10C0;
		KEEP (ArmDtcm.o(.dtcm))
		*(EXCLUDE_FILE(VMDtcm.o VMUndefinedThumbTable.o VMUndefinedArmTable.o) .dtcm)
		*(.dtcm.*)
		. = 0x332C;
		KEEP (VMUndefinedThumbTable.o(.dtcm))
		KEEP (VMUndefinedArmTable.o(.dtcm))
		. = 0x3F2C;
		KEEP (VMDtcm.o(.dtcm))
		. = ALIGN(4);
		__dtcm_end = ABSOLUTE(.);
	} >dtcm AT>vramab :dtcm = 0xff

	.itcm :
	{
		__itcm_lma = LOADADDR(.itcm);
		__itcm_start = ABSOLUTE(.);
		KEEP (vectors.o(.itcm))
		. = 0x20;
		KEEP (VMUndefined.o(.itcm))
		. = 0xB8;
		KEEP (VMIrq.o(.itcm))
		. = 0x378;
		KEEP (MemoryLoadRom.o(.itcm))
		. = 0x400;
		KEEP (ArmDispatch.o(.itcm))
		. = 0x450;
		KEEP (VMMode.o(.itcm))
		. = 0x7B4;
		KEEP (VMSwi.o(.itcm))
		KEEP (ArmLoadStoreShortRm.o(.itcm))
		KEEP (IoFallback.o(.itcm))
		KEEP (PrefetchAbort.o(.itcm))
		. = 0xC00;
		KEEP (ThumbDispatch.o(.itcm))
		KEEP (ThumbLoad.o(.itcm))
		KEEP (ThumbStore.o(.itcm))
		KEEP (ThumbLdmStm.o(.itcm))
		KEEP (ThumbLoadPc.o(.itcm))
		KEEP (MemCopy.o(.itcm))
		KEEP (JitThumbEnsureJitted.o(.itcm))
		KEEP (JitThumbUndefinedBx.o(.itcm))
		. = 0x2000;
		KEEP (ThumbGetRnRm.o(.itcm))
		KEEP (ArmLoadStoreShortRd.o(.itcm))
		KEEP (ArmSwpRd.o(.itcm))
		KEEP (GbaSoundAsm.o(.itcm))
		. = 0x3000;
		KEEP (ThumbGetRn.o(.itcm))
		*(.itcm)
		*.itcm*(.text .stub .text.*)
		. = ALIGN(4);
		__itcm_end = ABSOLUTE(.);
	} >itcm AT>vramab :itcm = 0xff

	.ewram.bss (NOLOAD):
	{
		__ewram_bss_start = ABSOLUTE(.);
		__ewram_bss_start__ = ABSOLUTE(.);
		*(.ewram.bss*)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
	} >ewram :NONE

	__ewram_bss_end = . ;
	__ewram_bss_end__ = . ;
	__heap_start = . ;

	.vramhi.bss (NOLOAD):
	{
		__vramhi_bss_start = ABSOLUTE(.);
		__vramhi_bss_start__ = ABSOLUTE(.);
		*(.vramhi.bss*)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
	} >vramhi :NONE

	__vramhi_bss_end = . ;
	__vramhi_bss_end__ = . ;

	.bss __bss_vma (NOLOAD):
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
	} >vrama :NONE

	__bss_end = . ;
	__bss_end__ = . ;

	_end = . ;
	__end__ = . ;
	PROVIDE (end = _end);

	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000 : { _stack = .; *(.stack) }
	/* These must appear regardless of  .  */
}
