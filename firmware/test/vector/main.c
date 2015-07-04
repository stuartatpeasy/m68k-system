#include <stdio.h>

#include "bvec.h"


u32 pred(void *element, u32 data)
{
	const int item = *((int *) element);

	printf("pred(): element=%u, data=%08x\n", item, (u32) data);

	if(item > 20000)
		return 1;

	return 0;
}


int main(int argc, char **argv)
{
	bvec_t bvec;
	int i, block_size = 64;

	bvec_init(block_size, sizeof(int), &bvec);

	/* write items */
	for(i = 0; i <= 1000; ++i)
	{
		int *item = bvec_grow(bvec);
		if(item == NULL)
		{
			printf("failed to allocate item %i\n", i);
			continue;
		}

		*item = i * 1000;
	}


	printf("Finished creating items.  Vector size is %u\n", bvec_size(bvec));
	printf("Item 0 is %u\n", *((int *) bvec->elements[0]));
	printf("Item 1 is %u\n", *((int *) bvec->elements[1]));
	printf("Item 2 is %u\n", *((int *) bvec->elements[2]));

	/* read items */
	bvec_iterate(bvec, pred, (u32) 0x12345678);
	
	bvec_destroy(&bvec);

	return 0;
}

