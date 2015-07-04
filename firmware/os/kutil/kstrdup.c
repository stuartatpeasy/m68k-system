/*
	kstrdup.c - implementation of kstrdup() [= libc strdup()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"
#include "kutil/kutil.h"
#include "memory/kmalloc.h"


s8 *kstrdup(ks8 *s)
{
	ku32 len = kstrlen(s);
	s8 *buf = kmalloc(len + 1);
	
	if(buf == NULL)
	{
		/* TODO: set error code */
		return 0;
	}
	
	kstrcpy(buf, s);

	return buf;
}

