#ifndef ELF_HARNESS_H
#define ELF_HARNESS_H
/*
	Harness for the ELF loader test code

	Part of the as-yet-unnamed MC68010 operating system


	Stuart Wallace, August 2015.
*/

/* Test harness runs on a little-endian platform, so ... */
#define B2L16(x)	((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define B2L32(x)	((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))
#define umalloc(x)	malloc(x)

/* Define these in order to avoid picking up the defs in the ayumos code FIXME remove*/
#define HAVE_SIZE_T	1
#define HAVE_PID_T	1

#define EINVAL 22			// FIXME remove
#define ENOMEM 12			// FIXME remove

#include <stdio.h>			// FIXME
#include <stdlib.h>			// FIXME
#include <string.h>			// FIXME
#include <strings.h>		// FIXME

#include "include/elf.h"

void elf_dump_hdr(const Elf32_Ehdr * const h);
void elf_dump_phdr(const Elf32_Phdr * const h);

#endif

