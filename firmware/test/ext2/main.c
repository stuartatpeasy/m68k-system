#include "harness.h"
#include "ext2.h"


int main(int argc, char **argv)
{
	if(block_init("fs"))
	{
		return 1;
	}

	ext2();

	block_shutdown();

	return 0;
}

