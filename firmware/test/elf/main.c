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

#include "harness.h"

#include "include/elf.h"


int main(int argc, char **argv)
{
	FILE *fp_elf, *fp_out;
	struct stat st;
	void *buf;
	exe_img_t img;

	if(argc != 3)
		error(1, 0, "Syntax: %s <elf_file> <mem_img>", argv[0]);
	
	if(stat(argv[1], &st))
		error(2, 0, "Failed to stat '%s'", argv[1]);

	if(!st.st_size)
		error(3, 0, "File '%s' is zero-length", argv[1]);

	if((fp_elf = fopen(argv[1], "r")) == NULL)
		error(4, errno, "Failed to open '%s'", argv[1]);

	if((fp_out = fopen(argv[2], "w")) == NULL)
		error(5, errno, "Failed to create '%s'", argv[2]);
	
	buf = malloc(st.st_size);
	if(!buf)
		error(6, 0, "Failed to allocate %d bytes", (int) st.st_size);
	
	if(fread(buf, 1, st.st_size, fp_elf) != st.st_size)
		error(7, errno, "Failed to read file '%s'", argv[1]);
	
	fclose(fp_elf);

	/* File image is in memory; attempt to parse and "load" it */
	elf_load_exe(buf, st.st_size, &img);

	printf("Local buffer: %p\nImage len: %u\nEntry point: %p\n",
			img.start, img.len, (void *) img.entry_point);

	if(fwrite(img.start, 1, img.len, fp_out) != img.len)
		error(8, errno, "Failed to write data to file '%s'", argv[2]);

	fclose(fp_out);
	

	return 0;
}
