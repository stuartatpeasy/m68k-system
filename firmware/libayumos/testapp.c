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
	int i;

	while(*p)
		sys_console_putchar(*p++);

	while(1)
	{
		for(i = 0; i < 100; ++i)
			sys_yield();

		sys_console_putchar('*');
	}

	/* sys_yield(); */

	/* Can't exit yet... */
	while(1)
		;

	return 0;
}
