/*
	makemap.c: build symbol table in binary format

	Part of ayumos


	(c) Stuart Wallace, August 2015.

	This program reads a symbol list in the format generated by the "nm" (1) tool and writes the
	symbols out in a binary format (see below for format description).  It reads from stdin and
	writes to stdout.
*/

#include <error.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#define MAX_LINE_LEN		(256)

/*
	Format of entries in an embedded symbol table:

		<addr>	4 bytes		symbol address
		<type>	1 byte		symbol type
		<len>	1 byte		length of this entire record
		<name>  var			zero-terminated name of symbol
	
	Entries are padded such that their total length is a multiple of four bytes.  The symbol table
	is terminated by an entry in which all fields are 0.
*/


enum symtype
{
	EXT_BSS  = 'B', 	/* The symbol is in the uninitialized data section (known as BSS). */
	INT_BSS  = 'b',
	EXT_DATA = 'D',		/* Initialised data (.data section) */
	INT_DATA = 'd',
	EXT_RO   = 'R',		/* Read-only (.rodata*) */
	INT_RO   = 'r',
	EXT_TEXT = 'T',		/* Text (code) section (.text) */
	INT_TEXT = 't'
};

typedef enum symtype symtype_t;

/* This struct will be tail-padded with \0s so that its size is a multiple of four bytes */
struct symentry
{
	uint32_t	addr;
	uint8_t		type;
	uint8_t		len;
	char 		name;		/* First character of name, a zero-terminated string */
} __attribute__((packed));

typedef struct symentry symentry_t;


/*
	checked_fputc() - a version of fputc() which bails out on error.
*/
int checked_fputc(int c, FILE *stream)
{
	int ret;

	ret = fputc(c, stream);
	if(ret == EOF)
		error(1, errno, "Write failed");

	return ret;
}


int main(int argc, char **argv)
{
	char name[MAX_LINE_LEN];
	FILE *fp_in, *fp_out;
	symentry_t sym;

	fp_in = stdin;
	fp_out = stdout;

	while(!feof(fp_in))
	{
		char *p;
		uint32_t len, padding;
		
		/*
			Format of the output of *nm <obj_file>:
		
				00f00be4 T ata_control
				[...]

			Longest symbol name allowable by our map format (which stores record length in a uint8)
			is:

				252 		<-- longest multiple-of-four representable in an unsigned 8-bit int
				- 4			<-- sizeof(u32) address field
				- 1			<-- sizeof(symtype_t) type field
				- 1			<-- sizeof(u8) length field
				- 1			<-- terminating '\0' at end of name
			  = 245 bytes
		*/

		bzero(name, sizeof(name) / sizeof(name[0]));
		if(fscanf(fp_in, "%08x %c %245s\n", &sym.addr, (char *) &sym.type, &name[0]) == 3)
		{
			if(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
				sym.addr = __builtin_bswap32(sym.addr);

			len = sizeof(sym.addr) + sizeof(sym.type) + sizeof(sym.len) + strlen(name) + 1; 
			padding = (4 - (len & 3)) & 3;

			sym.len = len + padding;
			
			for(p = (char *) &sym; p < &sym.name; ++p)
				checked_fputc(*p, fp_out);

			for(p = name; *p; ++p)
				checked_fputc(*p, fp_out);
			fputc(0, fp_out);

			for(; padding--;)
				checked_fputc(0, fp_out);
		}
	}

	/* Write end-of-table marker */
	sym.addr = 0;
	sym.type = 0;
	sym.len = 0;
	sym.name = 0;

	if(fwrite(&sym, sizeof(sym), 1, fp_out) < 1)
		error(1, errno, "Failed to write end-of-symbol-table marker");

	fflush(fp_out);

	return 0;
}
