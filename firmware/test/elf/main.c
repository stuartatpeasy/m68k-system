/*
	Test harness for ELF loader
*/

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Define these in order to avoid picking up the defs in the ayumos code */
#define HAVE_SIZE_T	1
#define HAVE_PID_T	1

#include "elf.h"


int main(int argc, char **argv)
{
	FILE *fp;
	struct stat st;
	void *buf;

	if(argc != 2)
		error(1, 0, "No ELF file specified");
	
	if(stat(argv[1], &st))
		error(2, 0, "Failed to stat '%s'", argv[1]);

	if(!st.st_size)
		error(3, 0, "File '%s' is zero-length", argv[1]);

	if((fp = fopen(argv[1], "r")) == NULL)
		error(4, errno, "Failed to open '%s'", argv[1]);
	
	buf = malloc(st.st_size);
	if(!buf)
		error(5, 0, "Failed to allocate %d bytes", (int) st.st_size);
	
	if(fread(buf, 1, st.st_size, fp) != st.st_size)
		error(6, errno, "Failed to read file '%s'", argv[1]);
	
	fclose(fp);

	/* File image is in memory; attempt to parse and "load" it */
	elf_load_exe(buf, st.st_size);

	return 0;
}
