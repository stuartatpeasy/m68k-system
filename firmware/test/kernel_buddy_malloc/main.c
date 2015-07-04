/*
	test_buddy.c: test harness for buddy memory allocator
*/

#include <stdlib.h>	// malloc(), free()
#include <stdio.h>
#include <err.h>
#include <errno.h>

#include "buddy.h"

#define MEM_SIZE		16	/* mem pool size as a power of two 			*/
#define MIN_ALLOC_UNIT	10	/* min allocatable block as a power of two	*/

signed char buddy_data[1 << (MEM_SIZE - MIN_ALLOC_UNIT)];

buddy_ctx b;


int main(int argc, char **argv)
{
	void *mem, *p[100];
	int i, sizes[] = {700, 4, 0, 1024, 38, 910, 2500, 17000, 500, 500, 500, 500, 500, 500, 500, 500};

	if((mem = malloc(1 << MEM_SIZE)) == NULL)	/* malloc() size must == 1 << POOL_SIZE */
		err(-ENOMEM, "failed to allocate memory\n");
	
	printf("[mem start = %p]\n\n", mem);

	buddy_init(&b, mem, 1 << MEM_SIZE, 1 << MIN_ALLOC_UNIT, buddy_data);

	for(i = 0; i < (sizeof(sizes) / sizeof(int)); ++i)
	{
		p[i] = buddy_malloc(&b, sizes[i]);
		printf("find_block(%d) = %p\n", sizes[i], p[i]);
		buddy_dump(&b);
	}

	buddy_free(&b, p[12]);
	buddy_dump(&b);
	buddy_free(&b, p[13]);
	buddy_dump(&b);
	buddy_free(&b, p[15]);
	buddy_dump(&b);
	buddy_free(&b, p[14]);
	buddy_dump(&b);

	buddy_free(&b, p[1]);
	buddy_dump(&b);
	buddy_free(&b, p[4]);
	buddy_dump(&b);
	buddy_free(&b, p[7]);
	buddy_dump(&b);
	buddy_free(&b, p[3]);
	buddy_dump(&b);
	buddy_free(&b, p[11]);
	buddy_dump(&b);
	buddy_free(&b, p[10]);
	buddy_dump(&b);
	buddy_free(&b, p[2]);
	buddy_dump(&b);
	buddy_free(&b, p[9]);
	buddy_dump(&b);
	buddy_free(&b, p[6]);
	buddy_dump(&b);
	buddy_free(&b, p[5]);
	buddy_dump(&b);
	buddy_free(&b, p[8]);
	buddy_dump(&b);
	buddy_free(&b, p[0]);
	buddy_dump(&b);

	printf("## free space = %u bytes\n", buddy_get_free_space(&b));
	printf("## used space = %u bytes\n", buddy_get_used_space(&b));

	free(mem);
	return 0;
}

