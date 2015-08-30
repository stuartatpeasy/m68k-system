/*
	First ever ayumos user-mode application
*/

#include "ayumos.h"

const char * const str = "Hello, World!";	/* Force a .rodata section in the app */
int x = 0x12345678;							/* Force a .data section */
int y;										/* Force a .bss section */


int main()
{
	sys_console_putchar('A');

	return 0;
}
