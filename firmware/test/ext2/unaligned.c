/*
	Unaligned read/write abstractions for block devices

	Part of the as-yet-unnamed MC68010 operating system.


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.
*/

#include "unaligned.h"


/*
	Perform an unaligned read of an arbitrary amount of data from a block device.

	N.B. This assumes that BLOCK_SIZE will always be a power of two.
*/
u32 unaligned_read(u32 start, u32 len, void *data)
{
	if(len)
	{
		u32 ret;
		void *buf = NULL;
		const void * const end = data + len;

		u32 block = start >> LOG_BLOCK_SIZE;

		/* is the starting offset aligned to a block? */
		start &= BLOCK_SIZE - 1;
		if(start)
		{
			/* starting offset is not aligned */
			buf = malloc(BLOCK_SIZE);
			if(buf == NULL)
				return ENOMEM;

			ret = block_read(block, buf);
			if(ret)
			{
				free(buf);
				return ret;
			}

			memcpy(data, buf + start, BLOCK_SIZE - start);

			data += BLOCK_SIZE - start;
		}

		/* these reads are definitely block-aligned */
		for(; (end - data) >= BLOCK_SIZE; data += BLOCK_SIZE)
		{
			ret = block_read(block++, data);
			if(ret)
			{
				if(buf)
					free(buf);

				return ret;
			}
		}

		/* is it necessary to read a partial final block? */
		if(end - data)
		{
			/* a buffer may already have been allocated by the block above */
			if(buf == NULL)
			{
				buf = malloc(BLOCK_SIZE);
				if(buf == NULL)
					return ENOMEM;
			}

			ret = block_read(block, buf);
			if(ret)
			{
				free(buf);
				return ret;
			}

			memcpy(data, buf, end - data);
		}

		if(buf)
			free(buf);
	}

	return SUCCESS;
}


/*
	Perform an unaligned write of an arbitrary amount of data to a block device.

	N.B. This assumes that BLOCK_SIZE will always be a power of two.
*/
u32 unaligned_write(u32 start, u32 len, const void *data)
{
	if(len)
	{
		u32 ret;
		void *buf = NULL;
		const void * const end = data + len;

		/* calculate first block to be read by rounding offset down to the nearest block */
		u32 block = start >> LOG_BLOCK_SIZE;

		/* is the starting offset aligned to a block? */
		start &= BLOCK_SIZE - 1;
		if(start)
		{
			/* starting offset is not aligned */
			buf = malloc(BLOCK_SIZE);
			if(buf == NULL)
				return ENOMEM;

			ret = block_read(block, buf);
			if(ret)
			{
				free(buf);
				return ret;
			}

			memcpy(buf + start, buf, BLOCK_SIZE - start);

			ret = block_write(block++, buf);
			if(ret)
			{
				free(buf);
				return ret;
			}

			data += BLOCK_SIZE - start;
		}

		/* these writes are definitely block-aligned */
		for(; (end - data) >= BLOCK_SIZE; data += BLOCK_SIZE)
		{
			ret = block_write(block++, data);
			if(ret)
			{
				if(buf)
					free(buf);

				return ret;
			}
		}

		/* is it necessary to write a partial final block? */
		if(end - data)
		{
			/* a buffer may already have been allocated by the nonaligned-starting-block code
			   above. */
			if(buf == NULL)
			{
				buf = malloc(BLOCK_SIZE);
				if(buf == NULL)
					return ENOMEM;

				ret = block_read(block, buf);
				if(ret)
				{
					free(buf);
					return ret;
				}

				memcpy(buf, data, end - data);

				ret = block_write(block, buf);
				if(ret)
				{
					free(buf);
					return ret;
				}
			}
		}

		if(buf)
			free(buf);
	}

	return SUCCESS;
}

