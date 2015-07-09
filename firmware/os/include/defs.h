#ifndef __INCLUDE_DEFS_H__
#define __INCLUDE_DEFS_H__
/*
	Miscellaneous definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <include/types.h>


/* API-like functions try to return zero on success and nonzero on failure */
#define SUCCESS			(0)
#define FAIL			(-1)

/* Directory separator character used in file paths */
#define DIR_SEPARATOR	('/')

/* Block device block size */
#define BLOCK_SIZE		(512)
#define LOG_BLOCK_SIZE	(9)

/* Maximum length of a name (i.e. a component in a file system path) */
#define NAME_MAX_LEN	(255)

/* "Tick rate" - number of timer interrupts per second */
#define TICK_RATE       (100)

/* Per-process stack size */
#define PROC_STACK_SIZE (16 * 1024)


extern u8 _sdata,       /* .data section start */
          _edata,       /* .data section end   */
          _sbss,        /* .bss  section start */
          _ebss,        /* .bss  section end   */
          _stext,       /* .text section start */
          _etext;       /* .text section end   */


#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif

