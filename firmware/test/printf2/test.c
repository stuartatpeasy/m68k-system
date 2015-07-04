#include "printf.h"
#include <stdio.h>


int main(int argc, char **argv)
{
	_printf("hello %'06.789llf there\n");
	printf("%%-04.5c: '%-04.5c'\n", 65);

	printf("% *.*f\n", 9, 6, 123456.789f);
	printf("%.1000lf\n", 1.0 / 3.0);

	return 0;
}

