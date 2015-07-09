MEMORY
{
    /* ROM lives at 0xf00000 after boot */
    rom : org = 0x00f00000, l = 1M

    /* Kernel RAM lives in the bottom 256K of the address space.
       User RAM extends above this. */
    ram : org = 0x00000000, l = 256K
}

SECTIONS
{
	. = 0x00000400;
	.data ALIGN(4):
    AT(ADDR(.text) + SIZEOF(.text))
	{
        _sdata = .;
        *(.data)
        _edata = .;
    }

	.bss ALIGN(4):
	{
        _sbss = .;
        *(.bss)
        _ebss = .;
    }

	. = 0x00f00000;
	.text :
	AT(ADDR(.text))
	{
        _stext = .;
        *(.text)
        _etext = .;
	}
}

ENTRY(_main)

STARTUP(startup.o)

OUTPUT_FORMAT(coff-m68k)

