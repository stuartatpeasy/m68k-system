/*
	First ever ayumos user-mode application
*/

#include "ayumos.h"

const char * const str = "Hello, World!";	/* Force a .rodata section in the app */
int x = 0x12345678;							/* Force a .data section */
int y;										/* Force a .bss section */


int main()
{
	const char *p = str;
	int i, j;

	while(*p)
		sys_console_putchar(*p++);

	for(j = 0; j < 10; ++j)
	{
		for(i = 0; i < 100; ++i)
			sys_yield();

		sys_console_putchar('*');
	}

	sys_exit(12345);

	return 0;
}
