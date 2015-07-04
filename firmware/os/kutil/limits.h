#ifndef __INCLUDE_KUTIL_LIMITS_H__
#define __INCLUDE_KUTIL_LIMITS_H__
/*
    limits.h - integer (and any other) limit constants

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#define CHAR_BIT			(8)
#define SCHAR_MAX			(127)
#define SCHAR_MIN			(-128)
#define UCHAR_MAX			(255)
#define MB_LEN_MAX			(4)

#define SHRT_MAX			(32767)
#define SHRT_MIN			(-SHRT_MAX - 1)
#define USHRT_MAX			(65535)

#define INT_MAX				(2147483647)
#define INT_MIN				(-INT_MAX - 1)
#define UINT_MAX			(4294967295U)

#define LONG_MAX			(2147483647L)
#define LONG_MIN			(-LONG_MAX -1L)
#define ULONG_MAX			(4294967295UL)

#define LLONG_MAX			(9223372036854775807LL)
#define LLONG_MIN			(−LLONG_MAX - 1LL)
#define ULLONG_MAX			(18446744073709551615ULL)

#endif // __INCLUDE_KUTIL_LIMITS_H__
