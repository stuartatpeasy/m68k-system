#ifndef __LIBCSW_ASSERT_H__
#define __LIBCSW_ASSERT_H__
/*
	assert.h - declaration of the assert() function

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-28.
*/

#ifdef NDEBUG
/*
	if NDEBUG is defined at this point, the assert() function should generate no code and do nothing.
*/

#define assert(x)

#else
/*
	if NDEBUG is not defined, the assert() function should trap errors as normal.
*/

void assert(int expression);

#endif
#endif

