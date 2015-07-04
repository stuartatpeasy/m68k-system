#ifndef __INCLUDE_LIMITS_H__
#define __INCLUDE_LIMITS_H__
/*
	Integer type limits

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#define CHAR_BIT			(8)
#define MB_LEN_MAX			(4)

#define S8_MAX			    (127)
#define S8_MIN			    (-128)
#define U8_MAX			    (255)

#define S16_MAX			    (32767)
#define S16_MIN			    (-S16_MAX - 1)
#define U16_MAX			    (65535)

#define S32_MAX				(2147483647)
#define S32_MIN				(-S32_MAX - 1)
#define U32_MAX			    (4294967295U)

#define S64_MAX			    (9223372036854775807LL)
#define S64_MIN			    (−LLONG_MAX - 1LL)
#define U64_MAX			    (18446744073709551615ULL)

#endif

