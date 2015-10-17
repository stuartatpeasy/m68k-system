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
	/* .text and .rodata* sections first - these live at the start of ROM. */
	. = 0x00f00000;

	.text :
	AT(ADDR(.text))
	{
        _stext = .;
        *(.text)
	} > rom

	_srodata = .;

	.rodata ALIGN(4):
	{
		*(.rodata*)
	} > rom

    _etext = .;

	. = 0x00000400;

	.data ALIGN(4):
    AT(ADDR(.rodata) + SIZEOF(.rodata))
	{
        _sdata = .;
        *(.data)
    }
    _edata = .;

	.bss ALIGN(4):
	AT(ADDR(.data) + SIZEOF(.data))
	{
        _sbss = .;
        *(.bss)
    }
    _ebss = .;
}

ENTRY(_main)

STARTUP(cpu/mc68000/startup.o)

