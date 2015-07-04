SECTIONS
{
	. = 0x00f00000;
	.text : { *(.text) }
	. = 0x00000400;
	.data : { *(.data) }
	.bss : { *(.bss) }
}

ENTRY(_main)

STARTUP(startup.o)

OUTPUT_FORMAT(coff-m68k)

