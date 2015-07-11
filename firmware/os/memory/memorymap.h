#ifndef __INCLUDE_MEMORYMAP_H__
#define __INCLUDE_MEMORYMAP_H__
/*
	Memory map declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, May 2012.


	Memory map
	==========

	00:0000 - 7f:ffff		8M		RAM
	---------------------------------------------------------------
		00:0000 - 00:03ff	      1K	CPU vector table
		00:0400 - 03:7fff	    223K	OS .data, .bss, slabs, heap
		03:8000 - 03:ffff	     32K	OS stack
		04:0000 - 7f:ffff      7936K    Application memory

	80:0000 - 9f:ffff		2M		[vacant]
	---------------------------------------------------------------

    a0:0000 - df:ffff       4M      Expansion cards
    ---------------------------------------------------------------
        a0:0000 - af:ffff         1M    Expansion slot 0
        b0:0000 - bf:ffff         1M    Expansion slot 1
        c0:0000 - cf:ffff         1M    Expansion slot 2
        d0:0000 - df:ffff         1M    Expansion slot 3

	e0:0000 - e0:ffff		1M		Memory-mapped peripherals
	---------------------------------------------------------------
		e0:0000 - e0:ffff	     64K	DUART
		e1:0000 - e1:ffff	     64K	RTC
		e2:0000 - e2:ffff	     64K	ATA channel 0
		e3:0000 - e3:ffff	     64K	ATA channel 1
        e4:0000 - 3f:ffff       768K    [vacant]

	f0:0000 - ff:ffff		1M		Operating system ROM
	---------------------------------------------------------------
*/


#define MEMMAP_START			(0)			/* probably redundant 				*/

#define VECTOR_TABLE_START		(0)			/*    0K: CPU vector table			*/
#define VECTOR_TABLE_END		(0x000400)

#define OS_STACK_BOTTOM			(0x038000)	/*	224K: OS stack (32K)			*/
#define OS_STACK_TOP			(0x040000)

#define APP_MEM_START			(0x040000)	/*  256K: Application RAM start	    */
#define APP_MEM_END				(0x800000)	/* 8192K: Top of RAM				*/

#endif

