#ifndef KLIBC_ASSERT_H_INC
#define KLIBC_ASSERT_H_INC

/*
	assert.h - declaration of the assert() function

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-28.
*/

#include "include/types.h"


#ifdef NDEBUG
/*
	if NDEBUG is defined at this point, the assert() function should generate no code and do nothing.
*/

#define assert(x)

#else
/*
	if NDEBUG is not defined, the assert() function should trap errors as normal.
*/

void assert(s32 expression);

#endif
#endif

