#include <stdio.h>
#include <stdlib.h>

#include "disasm.h"
/*
	main.c - MC68000 diassembler test harness

	(c) Stuart Wallace, September 2011.
*/


int main(int argc, char **argv)
{
	int j;
	FILE *fp;
	long size, offset = 0, num_bytes = -1;
	unsigned short *code;

	char str[8192], instr_printed;


	if((argc < 2) || (argc > 4))
	{
		fprintf(stderr, "Syntax: %s <binary file> [<offset> [<num_bytes>]]\n", argv[0]);
		return 1;
	}

	if(!(fp = fopen(argv[1], "r")))
	{
		fprintf(stderr, "Cannot open input file '%s' for reading\n", argv[1]);
		return 2;
	}

	if(argc > 2)
	{
		offset = strtol(argv[2], NULL, 0);
		if((offset < 0) || (offset & 1))
		{
			fprintf(stderr, "Invalid offset: cannot be negative or an odd number\n");
			return 3;
		}

		if(argc > 3)
		{
			num_bytes = strtol(argv[3], NULL, 0);
			if((num_bytes < 0) || (num_bytes & 1))
			{
				fprintf(stderr, "Invalid num_bytes: cannot be negative or an odd number \n");
				return 3;
			}
		}
	}

	// Determine length of input file, allocate a buffer and read the file into it
	if(fseek(fp, 0, SEEK_END))
	{
		perror("fseek() failed");
		return 2;
	}

	if((size = ftell(fp)) == -1)
	{
		perror("ftell() failed");
		return 2;
	}

	if((offset > (size - 2)))
	{
		fprintf(stderr, "Offset %ld is beyond end of input data (%ld)\n", offset, size);
		return 3;
	}

	if(num_bytes == -1)
		num_bytes = size;

	if((offset + num_bytes) > size)
		num_bytes = size - offset;

	if(fseek(fp, 0, SEEK_SET))
	{
		perror("fseek() failed");
		return 2;
	}

	if((code = malloc((size + 1) & ~1)) == NULL)
	{
		perror("malloc() failed");
		return 3;
	}

	if(fread(code, 1, size, fp) != size)
	{
		perror("fread() failed");
		return 2;
	}
	fclose(fp);

	unsigned short *p = code + (offset >> 1), *q = p;
	for(; p < (code + ((offset + num_bytes) >> 1));)
	{
		instr_printed = 0;
		disassemble(&p, str);
		while(q < p)
		{
			printf("%4x:   ", sizeof(unsigned short) * (q - code));
			for(j = 0; j < 3; ++j)
				if(q < p)
				{
					printf("%04x ", HTOP_SHORT(*q));
					++q;
				}
				else
					printf("     ");

			if(!instr_printed)
			{
				printf(" %s\n", str);
				instr_printed = 1;
			}
			else puts("");
		}
	}

	return 0;
}

