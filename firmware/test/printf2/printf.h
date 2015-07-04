#ifndef __LIBCSW_PRINTF_H__
#define __LIBCSW_PRINTF_H__

#include <stdarg.h>
#include <stddef.h>

#define PRINTF_FLAG_ALTERNATE_FORM	(0x01)
#define PRINTF_FLAG_ZERO_PAD		(0x02)
#define PRINTF_FLAG_LEFT_JUSTIFY	(0x04)
#define PRINTF_FLAG_LEADING_BLANK	(0x08)
#define PRINTF_FLAG_FORCE_SIGN		(0x10)
#define PRINTF_FLAG_GROUP_THOUSANDS	(0x20)
#define PRINTF_FLAG_UPPERCASE		(0x40)

enum printf_len_t
{
	PRINTF_LEN_NONE = 0,
	PRINTF_LEN_H,
	PRINTF_LEN_HH,
	PRINTF_LEN_L,
	PRINTF_LEN_LD,
	PRINTF_LEN_LL,
	PRINTF_LEN_INTMAX_T,
	PRINTF_LEN_SIZE_T,
	PRINTF_LEN_PTRDIFF_T
};

enum printf_conversion_t
{
	PRINTF_CONV_INVALID = 0,
	PRINTF_CONV_SDEC,
	PRINTF_CONV_UOCT,
	PRINTF_CONV_UDEC,
	PRINTF_CONV_HEX,
	PRINTF_CONV_DOUBLE_STDFORM,
	PRINTF_CONV_DOUBLE_NORMAL,
	PRINTF_CONV_DOUBLE_AUTO,
	PRINTF_CONV_DOUBLE_HEX,
	PRINTF_CONV_CHAR,
	PRINTF_CONV_STRING,
	PRINTF_CONV_PTR,
	PRINTF_CONV_COUNT,
	PRINTF_CONV_PERCENT
};

struct printf_format
{
	unsigned int				flags;
	unsigned int				width;
	unsigned int				precision;
	enum printf_len_t			length;
	enum printf_conversion_t	conversion;
};

int _vprintf(const char *format, va_list ap);
int _vsprintf(char *str, const char *format, va_list ap);
int _vsnprintf(char *str, size_t size, const char *format, va_list ap);
int _printf(const char *format, ...);
int _sprintf(char *str, const char *format, ...);
int _snprintf(char *str, size_t size, const char *format, ...);

#ifdef DEBUG

void dump_format_spec(const struct printf_format * const f);

#endif

#endif

