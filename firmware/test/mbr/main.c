/*
	Test harness and main() for MBR-reading code

	Stuart Wallace, July 2012.
*/

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/defs.h"		/* from OS code */
#include "mbr.h"

extern FILE *fp;

int main(int argc, char **argv)
{
	int ret;

	if(argc != 2)
	{
		fprintf(stderr, "Syntax: %s <file>\n", argv[0]);
		return 1;
	}

	if((fp = fopen(argv[1], "r")) == NULL)
	{
		err(2, "Cannot open '%s'", argv[1]);
	}

	ret = mbr_read();

	fclose(fp);

	if(ret)
	{
		fprintf(stderr, "MBR read failed\n");
		return 3;
	}
	else
	{
		puts("MBR read succeeded");
	}

	return 0;
}
