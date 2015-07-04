#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int i;
	for(i = 0; i < 256; i++)
	{
		printf("%3i %s\n", i, strerror(i));
	}

	printf("*** %s ****\n", strerror(ENOENT));
}

