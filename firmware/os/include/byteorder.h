#ifndef INCLUDE_BYTEORDER_H_INC
#define INCLUDE_BYTEORDER_H_INC
/*
	Byte-order conversion macros

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.


	Format of conversion macros:

		<endianness>2<endianness><size>(x)		...or
		<endianness>HW2<endianness>32(x)

	<endianness> is one of "LE" (little-endian), "BE" (big-endian), or "N" (native).  Native
	endianness is big-endian on m68k machines and little-endian on Intel machines.

	<size> is 16 (half-word) or 32 (word) bits.

	The second form is only valid for 32-bit operands.  It assumes that the endianness of the
	bytes within each half of the operand is correct; it swaps the 16-bit halves if the platform
	endianness does not match the operand.  For example, on a big-endian platform:

		y = LEHW2N32(0x11223344);

	'y' would have the value 0x33441122.  These macros are useful when dealing with 32-bit data
	items read from an ATA device.
*/

#include <kernel/cpu.h>

#if defined(TARGET_BIGENDIAN)

#define LE2N16(x)	__builtin_bswap16(x)
#define LE2N32(x)	__builtin_bswap32(x)
#define BE2N16(x)	(x)
#define BE2N32(x)	(x)

#define LEHW2N32(x)	wswap_32(x)
#define BEHW2N32(x)	(x)

#elif defined(TARGET_LITTLEENDIAN)

#define LE2N16(x)	(x)
#define LE2N32(x)	(x)
#define BE2N16(x)	__builtin_bswap16(x)
#define BE2N32(x)	__builtin_bswap32(x)

#define LEHW2N32(x) (x)
#define BEHW2N32(x)	wswap_32(x)

#else
#error Target endianness not defined
#endif

#define N2LE16(x)	LE2N16(x)
#define N2LE32(x)	LE2N32(x)

#define N2BE16(x)	BE2N16(x)
#define N2BE32(x)	BE2N32(x)

#define N2LEHW32(x)	LEHW2N32(x)
#define N2BEHW32(x)	BEHW2N32(x)

#endif

