#include <stdio.h>
#include <stdlib.h>

#include "harness.h"

/*
	This file implements block read/write functions on a file, instead of a real block device.
	The interface is the same as on the target hardware, but a local file is used instead of
	an actual hard drive.

*/

FILE *g_blockdev = NULL;
u32 g_len = 0;


u32 block_init(const char * const filename)
{
	FILE *f;
	unsigned long file_size;

	f = fopen(filename, "r");
	if(!f)
	{
		perror("fopen()");
		return 1;		/* failed to open file */
	}

	if(fseek(f, 0, SEEK_END) == -1)
	{
		perror("fseek()");
		return 1;		/* failed to seek to end of file */
	}

	file_size = ftell(f);
	if(file_size == -1)
	{
		perror("ftell()");
		return 1;		/* failed to read file offset */
	}

	rewind(f);

	if(!block_shutdown())
	{
		return 1;		/* failed to deactivate existing block device file */
	}

	g_blockdev = f;
	g_len = file_size;

	return 0;	/* success */
}


u32 block_shutdown()
{
	if(g_blockdev != NULL)
	{
		if(fclose(g_blockdev) == EOF)
		{
			perror("fclose()");
			return 0;		/* fclose() failed */
		}

		g_blockdev = NULL;
		g_len = 0;
	}

	return 1;	/* success */
}


u32 block_read(ku32 block_num, void* const buf)
{
	if((block_num * BLOCK_SIZE) >= g_len)
	{
		return 1;	/* invalid range */
	}

	if(fseek(g_blockdev, block_num * BLOCK_SIZE, SEEK_SET) == -1)
	{
		perror("fseek()");
		return 1;	/* seek failed */
	}

	if(fread(buf, BLOCK_SIZE, 1, g_blockdev) != 1)
	{
		perror("fread()");
		return 1;	/* read failed */
	}

	return 0;
}


u32 block_read_multi(u32 first_block_num, u32 num_blocks, void * buf)
{
	for(; num_blocks--; buf = (u8 *) buf + BLOCK_SIZE)
	{
		ku32 ret = block_read(first_block_num++, (u8 *) buf);
		if(ret)
			return ret;		/* block_read() failed */
	}

	return 0;
}


u32 block_write(ku32 block_num, const void* const buf)
{
	if((block_num * BLOCK_SIZE) >= g_len)
	{
		return 1;	/* invalid range */
	}

	if(fseek(g_blockdev, block_num * BLOCK_SIZE, SEEK_SET) == -1)
	{
		perror("fseek()");
		return 1;	/* seek failed */
	}

	if(fwrite(buf, BLOCK_SIZE, 1, g_blockdev) != 1)
	{
		perror("fread()");
		return 1;	/* write failed */
	}

	return 0;
}


u32 block_write_multi(u32 first_block_num, u32 num_blocks, const void * buf)
{
	for(; num_blocks--; buf = (u8 *) buf + BLOCK_SIZE)
	{
		ku32 ret = block_write(first_block_num++, (u8 *) buf);
		if(ret)
			return ret;		/* block_write() failed */
	}

	return 0;
}

