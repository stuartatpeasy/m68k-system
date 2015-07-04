/*
	vprintf.c - implementation of vprintf(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "limits.h"

/*
	Size of the initial buffer used by vprintf() (and therefore printf()).  If this buffer is
	insufficient, a larger one will be allocated.
*/
#define PRINTF_INITIAL_BUFFER_SIZE	(128)


int vprintf(const char *format, va_list ap)
{
	size_t buf_size = PRINTF_INITIAL_BUFFER_SIZE;
	char *buf;
	int n;

	while(1)
	{
		buf = (char *) malloc(buf_size);
		assert(buf != NULL);
		n = vsnprintf(buf, buf_size, format, ap);

		if(n == -1)
			return n;

		if(n < buf_size)
		{
			put(buf);
			free(buf);
			return n;
		}

		free(buf);
		buf_size = n + 1;
	}
}

