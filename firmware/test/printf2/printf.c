#include <stdarg.h>
#include <stdio.h>
#include <limits.h>			/* for INT_MAX */
#include "printf.h"


#ifdef DEBUG

void dump_format_spec(const struct printf_format* const f)
{
	char buf[32];
	int i = 0;

	buf[i++] = '%';

	if(f->flags & PRINTF_FLAG_ALTERNATE_FORM)	buf[i++] = '#';
	if(f->flags & PRINTF_FLAG_GROUP_THOUSANDS)	buf[i++] = '\'';
	if(f->flags & PRINTF_FLAG_LEADING_BLANK)	buf[i++] = ' ';
	if(f->flags & PRINTF_FLAG_LEFT_JUSTIFY)		buf[i++] = '-';
	if(f->flags & PRINTF_FLAG_FORCE_SIGN)		buf[i++] = '+';
	if(f->flags & PRINTF_FLAG_ZERO_PAD)			buf[i++] = '0';

	i += sprintf(buf + i, "%u.%u", f->width, f->precision);
	
	if(f->length == PRINTF_LEN_H)				buf[i++] = 'h';
	else if(f->length == PRINTF_LEN_L)			buf[i++] = 'l';
	else if(f->length == PRINTF_LEN_LD)			buf[i++] = 'L';
	else if(f->length == PRINTF_LEN_INTMAX_T)	buf[i++] = 'j';
	else if(f->length == PRINTF_LEN_SIZE_T)		buf[i++] = 'z';
	else if(f->length == PRINTF_LEN_PTRDIFF_T)	buf[i++] = 't';
	else if(f->length == PRINTF_LEN_HH)
	{
		buf[i++] = 'h';
		buf[i++] = 'h';
	}
	else if(f->length == PRINTF_LEN_LL)
	{
		buf[i++] = 'l';
		buf[i++] = 'l';
	}

	switch(f->conversion)
	{
		case PRINTF_CONV_SDEC:
			buf[i++] = 'd';
			break;

		case PRINTF_CONV_UOCT:
			buf[i++] = 'o';
			break;

		case PRINTF_CONV_UDEC:
			buf[i++] = 'u';
			break;

		case PRINTF_CONV_HEX:
			buf[i++] = (f->flags & PRINTF_FLAG_UPPERCASE) ? 'X' : 'x';
			break;

		case PRINTF_CONV_DOUBLE_STDFORM:
			buf[i++] = (f->flags & PRINTF_FLAG_UPPERCASE) ? 'E' : 'e';
			break;

		case PRINTF_CONV_DOUBLE_NORMAL:
			buf[i++] = (f->flags & PRINTF_FLAG_UPPERCASE) ? 'F' : 'f';
			break;

		case PRINTF_CONV_DOUBLE_AUTO:
			buf[i++] = (f->flags & PRINTF_FLAG_UPPERCASE) ? 'G' : 'g';
			break;

		case PRINTF_CONV_DOUBLE_HEX:
			buf[i++] = (f->flags & PRINTF_FLAG_UPPERCASE) ? 'A' : 'a';
			break;

		case PRINTF_CONV_CHAR:
			buf[i++] = 'c';
			break;

		case PRINTF_CONV_STRING:
			buf[i++] = 's';
			break;

		case PRINTF_CONV_PTR:
			buf[i++] = 'p';
			break;

		case PRINTF_CONV_COUNT:
			buf[i++] = 'n';
			break;

		default:
			buf[i++] = '?';
			break;
	}

	buf[i++] = '\0';
	printf("%s", buf);
}

#endif


void formatstr(const char **s, struct printf_format * const f)
{
	/* Flag characters */
	f->flags = 0;
	for(++*s; **s; ++*s)
		if(**s == '%')
		{
			f->conversion = PRINTF_CONV_PERCENT;
			return;
		}
		else if(**s == '#')  f->flags |= PRINTF_FLAG_ALTERNATE_FORM;
		else if(**s == '0')  f->flags |= PRINTF_FLAG_ZERO_PAD;
		else if(**s == '-')  f->flags |= PRINTF_FLAG_LEFT_JUSTIFY;
		else if(**s == ' ')  f->flags |= PRINTF_FLAG_LEADING_BLANK;
		else if(**s == '+')  f->flags |= PRINTF_FLAG_FORCE_SIGN;
		else if(**s == '\'') f->flags |= PRINTF_FLAG_GROUP_THOUSANDS;
		else break;

	/* Field width */
	f->width = 0;
	for(; (**s >= '0') && (**s <= '9'); ++*s)
		f->width = (f->width * 10) + (**s - '0');

	/* Precision */
	f->precision = 0;
	if(**s == '.')
		for(++*s; (**s >= '0') && (**s <= '9'); ++*s)
			f->precision = (f->precision * 10) + (**s - '0');

	/* Length modifier */
	f->length = 0;
	if(**s == 'h')      f->length = PRINTF_LEN_H;
	else if(**s == 'l') f->length = PRINTF_LEN_L;
	else if(**s == 'L') f->length = PRINTF_LEN_LD;
	else if(**s == 'j') f->length = PRINTF_LEN_INTMAX_T;
	else if(**s == 'z') f->length = PRINTF_LEN_SIZE_T;
	else if(**s == 't') f->length = PRINTF_LEN_PTRDIFF_T;

	if(f->length)
	{
		++*s;
		if((**s == 'h') && (f->length == PRINTF_LEN_H))
		{
			f->length = PRINTF_LEN_HH;
			++*s;
		}
		else if((**s == 'l') && (f->length == PRINTF_LEN_L))
		{
			f->length = PRINTF_LEN_LL;
			++*s;
		}
	}

	/* Conversion specifier */
	f->conversion = 0;
	switch(**s)
	{
		case 'd':
		case 'i': f->conversion = PRINTF_CONV_SDEC; break;

		case 'o': f->conversion = PRINTF_CONV_UOCT; break;

		case 'u': f->conversion = PRINTF_CONV_UDEC; break;

		case 'X': f->flags |= PRINTF_FLAG_UPPERCASE;
		case 'x': f->conversion = PRINTF_CONV_HEX; break;

		case 'E': f->flags |= PRINTF_FLAG_UPPERCASE;
		case 'e': f->conversion = PRINTF_CONV_DOUBLE_STDFORM; break;

		case 'F': f->flags |= PRINTF_FLAG_UPPERCASE;
		case 'f': f->conversion = PRINTF_CONV_DOUBLE_NORMAL; break;

		case 'G': f->flags |= PRINTF_FLAG_UPPERCASE;
		case 'g': f->conversion = PRINTF_CONV_DOUBLE_AUTO; break;

		case 'A': f->flags |= PRINTF_FLAG_UPPERCASE;
		case 'a': f->conversion = PRINTF_CONV_DOUBLE_HEX; break;

		case 'c': f->conversion = PRINTF_CONV_CHAR; break;

		case 's': f->conversion = PRINTF_CONV_STRING; break;

		case 'p': f->conversion = PRINTF_CONV_PTR; break;

		case 'n': f->conversion = PRINTF_CONV_COUNT; break;

		default: f->conversion = PRINTF_CONV_INVALID; break;
	}
}

/*
	%s
		# - undefined
		0 - undefined


*/

int _vprintf(const char *format, va_list ap)
{
	char c;
	struct printf_format f;

	for(; (c = *format); ++format)
	{
		if(c != '%')
			putchar(c);
		else
		{
			formatstr(&format, &f);
			putchar('[');
			dump_format_spec(&f);
			putchar(']');
		}
	}

	return 0;

}


int _vsprintf(char *str, const char *format, va_list ap)
{
	return _vsnprintf(str, INT_MAX, format, ap);
}


int _vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return 0;	/* TODO */
}


int _printf(const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = _vprintf(format, ap);
	va_end(ap);

	return n;
}


int _sprintf(char *str, const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = _vsnprintf(str, INT_MAX, format, ap);
	va_end(ap);

	return n;
}


int _snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = _vsnprintf(str, size, format, ap);
	va_end(ap);

	return n;
}

