/*
	Test harness for MBR-reading code

	Stuart Wallace, July 2012.
*/

#include <stdio.h>

#define SUCCESS 0
#define FAIL 1

#define SECTOR_SIZE (512)

FILE *fp;


int read_sector(const unsigned int num, void *buf)
{
	if(fseek(fp, num * SECTOR_SIZE, SEEK_SET))
		return FAIL;

	if(fread(buf, 1, SECTOR_SIZE, fp) != SECTOR_SIZE)
		return FAIL;

	return SUCCESS;
}

