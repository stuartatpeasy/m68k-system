#ifndef KERNEL_INCLUDE_KSYM_H_INC
#define KERNEL_INCLUDE_KSYM_H_INC
/*
	Kernel symbol table lookup functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


typedef enum symtype
{
	EXT_BSS  = 'B', 	/* The symbol is in the uninitialized data section (known as BSS). */
	INT_BSS  = 'b',
	EXT_DATA = 'D',		/* Initialised data (.data section) */
	INT_DATA = 'd',
	EXT_RO   = 'R',		/* Read-only (.rodata*) */
	INT_RO   = 'r',
	EXT_TEXT = 'T',		/* Text (code) section (.text) */
	INT_TEXT = 't'
} symtype_t;

/* This struct will be tail-padded with \0s so that its size is a multiple of four bytes */
typedef struct symentry
{
	void *  addr;
	u8      type;
	u8      len;
	char    name;		/* First character of name, a zero-terminated string */
} __attribute__((packed)) symentry_t;


s32 ksym_find_by_name(const char * const name, symentry_t **ent);
s32 ksym_find_nearest_prev(void *addr, symentry_t **ent);
s32 ksym_format_nearest_prev(void *addr, char *buf, u32 buf_len);

#endif
