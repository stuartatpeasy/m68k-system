/*
	kputs.c: definition of the *printf() family of functions, used in kernel debugging

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, June 2015.
*/

#include <stdarg.h>
#include "duart.h"
#include "kutil/kutil.h"
#include "include/limits.h"
#include "memory/kmalloc.h"

/*
	Size of the initial buffer used by vprintf() (and therefore printf()).  If this buffer is
	insufficient, a larger one will be allocated.
*/
#define PRINTF_INITIAL_BUFFER_SIZE	(128)

#define PF_HALF					(0x01)
#define PF_LONG					(0x02)
#define PF_LEADING_ZERO			(0x04)
#define PF_HAVE_FIELD_WIDTH		(0x08)
#define PF_FIELD_DOT_SEEN		(0x10)
#define PF_NUM_OUTPUT_STARTED	(0x20)

static ku32 powers_of_ten[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
                                10000000, 100000000, 1000000000};
static ku16 npowers = sizeof(powers_of_ten) / sizeof(ku32);


/*
    printf()
*/
s32 printf(const char *format, ...)
{
	va_list ap;
	s32 n;

	va_start(ap, format);
	n = vprintf(format, ap);
	va_end(ap);

	return n;
}


/*
    snprintf()
*/
s32 snprintf(char *str, u32 size, const char *format, ...)
{
	va_list ap;
	s32 n;

	va_start(ap, format);
	n = vsnprintf(str, size, format, ap);
	va_end(ap);

	return n;
}


/*
    sprintf()
*/
s32 sprintf(char *str, const char *format, ...)
{
	va_list ap;
	s32 n;

	va_start(ap, format);
	n = vsnprintf(str, S32_MAX, format, ap);
	va_end(ap);

	return n;
}


/*
    vprintf()
*/
s32 vprintf(const char *format, va_list ap)
{
	s32 buf_size = PRINTF_INITIAL_BUFFER_SIZE;
	char *buf;
	s32 n;

	while(1)
	{
		buf = (char *) kmalloc(buf_size);
		n = vsnprintf(buf, buf_size, format, ap);

		if(n == -1)
			return n;

		if(n < buf_size)
		{
			kput(buf);
			kfree(buf);
			return n;
		}

		kfree(buf);
		buf_size = n + 1;
	}
}


/*
    vsprintf()
*/
s32 vsprintf(char *str, const char *format, va_list ap)
{
	return vsnprintf(str, S32_MAX, format, ap);		/* Assume an unlimited buffer */
}


/*
    vsnprintf()
*/


s32 vsnprintf(char *str, u32 size, const char *format, va_list ap)
{
	s32 n = 0, flags, ndigits, ndecimals;
	u32 u;
	const char *pc;
	const u32 size_ = size - 1;		/* number of bytes available in buffer, reserving one byte for the trailing \0 */
	char ch;

	while((ch = *format))
	{
		if(ch == '%')
		{
			ch = *++format;
			if(!ch)
				break;
			else if(ch == '%')
			{
				if(n < size_)
					str[n] = '%';
				++n;
				break;
			}

			/* look for field-width specifier */
			flags = ndigits = ndecimals = 0;
			while(((ch >= '0') && (ch <= '9')) || (ch == '.'))
			{
				if(ch == '.')
				{
					flags |= PF_FIELD_DOT_SEEN;
				}
				else
				{
					if((ch == '0') && !(flags & PF_HAVE_FIELD_WIDTH))
						flags |= PF_LEADING_ZERO;

					flags |= PF_HAVE_FIELD_WIDTH;

					if(flags & PF_FIELD_DOT_SEEN)
					{
						ndecimals = (ndecimals << 3) + (ndecimals << 1);
						ndecimals += (ch - '0');
					}
					else
					{
						ndigits = (ndecimals << 3) + (ndecimals << 1);
						ndigits += (ch - '0');
					}
				}
				ch = *++format;
			}

			if(ch == 'l')
			{
				flags |= PF_LONG;
				ch = *++format;
			}
			else if(ch == 'h')
			{
				flags |= PF_HALF;
				ch = *++format;
			}

			if(ch == 's')
			{
				/* put string */
				for(pc = va_arg(ap, const char *); *pc; ++pc)
					if(n < size_)
						str[n++] = *pc;
			}
			else if(ch == 'c')
			{
				if(n < size_)
					str[n] = (char) va_arg(ap, int);
				++n;
			}
			else if((ch == 'd') || (ch == 'i') || (ch == 'u'))
			{
				u32 required_digits, numeral;

				/* put decimal */
				u = va_arg(ap, u32);

				if((flags & PF_HAVE_FIELD_WIDTH) && !ndigits && !u)
				{
					/* Explicit zero-size field and zero-value arg; silently skip. */
				}
				else
				{
					// if arg is -ve, convert it to +ve and output a '-'
					if((ch != 'u') && (u & 0x80000000))
					{
						u = ~u + 1;
						if(n < size_)
							str[n] = '-';
						++n;
					}

					/*
						calculate the number of digits required to display the number. override the width specifier
						if it does not allow sufficient digits.
					*/
					for(required_digits = 0; (required_digits < npowers) && (powers_of_ten[required_digits] <= u); ++required_digits) ;
					if(!required_digits)
						required_digits = 1;

					if(required_digits > ndigits)
						ndigits = required_digits;

					/* if leading zeroes or padding is enabled, and ndigits > required_digits, output leading zeroes */
					if(ndigits > required_digits)
						for(ch = (flags & PF_LEADING_ZERO) ? '0' : ' '; ndigits > required_digits; --ndigits)
						{
							if(n < size_)
								str[n] = ch;
							++n;
						}

					for(--ndigits; ndigits >= 0; --ndigits)
					{
						numeral = 0;
						while(u >= powers_of_ten[ndigits])
						{
							++numeral;
							u -= powers_of_ten[ndigits];
						}

						if(n < size_)
							str[n] = '0' + numeral;
						++n;
					}
				}
			}
			else if((ch == 'x') || (ch == 'X') || (ch == 'p'))
			{
				const char *hex;
				u32 val;
				s32 nybble = 7;

				/* put hex val / ptr */
				u = va_arg(ap, u32);

				if((ch == 'p') && ((n + 1) < size_))
				{
					if(!ndigits)
						ndigits = 8;

					str[n++] = '0';
					str[n++] = 'x';
					flags |= PF_LEADING_ZERO;
				}

				if(ch == 'X')
					hex = "0123456789ABCDEF";
				else
					hex = "0123456789abcdef";

				if(!ndigits)
					ndigits = 1;
				else
				{
					if(ndigits > 8)
						ndigits = 8;

					nybble = --ndigits;
					flags |= PF_NUM_OUTPUT_STARTED;
				}

				for(; nybble >= 0; --nybble)
				{
					val = (u >> (nybble << 2)) & 0xf;
					if(val || !nybble)
					{
						flags |= PF_NUM_OUTPUT_STARTED | PF_LEADING_ZERO;
						if(n < size_)
							str[n] = hex[val];
						++n;
					}
					else if(flags & PF_NUM_OUTPUT_STARTED)
					{
						if(n < size_)
							str[n] = (flags & PF_LEADING_ZERO) ? '0' : ' ';
						++n;
					}
				}
			}
		} /* if(ch == '%') */
		else if(ch == '\\')
		{
			/* TODO: support e.g. \x<hex>, etc. */
			if((ch = *++format))
			{
				if(n < size_)
					switch(ch = *++format)
					{
						case 'n':		str[n] = '\n';		break;
						case 'r':		str[n] = '\r';		break;
						case 't':		str[n] = '\t';		break;
						case '0':		str[n] = '\0';		break;

						default:		str[n] = ch;		break;
					}

				++n;
			}
		}
		else
		{
			if(n < size_)
				str[n] = ch;
			++n;
		}

		++format;
	}

	/* Write terminating \0 */
	str[(n <= size_) ? n : size_] = '\0';

	return n;
}
