#ifndef _EXT2_H_
#define _EXT2_H_
/*
	Unaligned read/write abstractions for block devices

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.
*/

#include <stdlib.h>		/* FIXME for malloc() - substitute kmalloc() header, or whatever */
#include <string.h>		/* FIXME for memcpy() - substitute */
#include "harness.h"	/* FIXME remove */

#include "include/defs.h"
#include "include/errno.h"
#include "include/types.h"


u32 unaligned_read(u32 start, u32 len, void *data);
u32 unaligned_write(u32 start, u32 len, const void *data);

#endif

