/*
	Test module for kernel memory allocator

	(c) Stuart Wallace, 13th August 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "malloc.h"

os_heap_t heap;


void dump(void)
{
	const unsigned int bytes_per_line = 32;
	unsigned int i;
	unsigned int *p = (unsigned int *) heap.start;
	unsigned char *c = (unsigned char *) heap.start;

	while(p < (unsigned int *) (heap.start + heap.size))
	{
		printf("%06x:  ", sizeof(unsigned int) * (p - (unsigned int *) heap.start));
		for(i = (bytes_per_line / sizeof(unsigned int)); i--; ++p)
			printf("%08x ",  *p);

		printf("   ");

		for(i = bytes_per_line; i--; ++c)
			printf("%c", isprint(*c) ? *c : '.');

		puts("");
	}
}


void *test_malloc(const unsigned int size)
{
	void *p = os_malloc(&heap, size);
	printf("* malloc(%u) = %08x (%s)\n", size, p - heap.start, p ? "success" : "FAILED");
	printf("* os_get_heap_free_memory() = %u\n", os_get_heap_free_memory(&heap));
	return p;
}


void *test_realloc(const void *ptr, const unsigned int size)
{
	void *p = os_realloc(&heap, ptr, size);
	printf("* realloc(%08x, %u) = %08x (%s)\n", ptr - heap.start, size, p - heap.start,
											   p ? "success" : "FAILED");
	printf("* os_get_heap_free_memory() = %u\n", os_get_heap_free_memory(&heap));
	return p;
}


void *test_calloc(const unsigned int nmemb, const unsigned int size)
{
	void *p = os_calloc(&heap, nmemb, size);
	printf("* calloc(%u, %u) = %08x (%s)\n", nmemb, size, p - heap.start, p ? "success" : "FAILED");
	printf("* os_get_heap_free_memory() = %u\n", os_get_heap_free_memory(&heap));
	return p;
}


int main()
{
	const unsigned int heap_size = 1024;
	unsigned int i;

	char *mb = malloc(heap_size + 4);
	char *memblock = (char *) ((unsigned int) (mb + 3) & ~3);

	for(i = 0; i < heap_size; memblock[i++] = 0x01);

	void *p[10];

	heap.start = memblock;
	heap.size = heap_size;

	//
	// Tests start here
	//

	os_heap_init(&heap);

	p[0] = test_malloc(3);
	p[1] = test_malloc(15);
	p[2] = test_malloc(200);
	p[3] = test_malloc(1);

	strcpy(p[2], "Hello!");
	
	p[2] = test_realloc(p[2], 250);
	p[4] = test_calloc(100, 1);

	dump();

	return 0;
}

