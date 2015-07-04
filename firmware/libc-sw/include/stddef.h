#ifndef __LIBCSW_STDDEF_H__
#define __LIBCSW_STDDEF_H__
/*
	stddef.h - definitions of various macros

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/


/*
	NULL
*/
#ifdef NULL
#undef NULL
#endif
#define NULL (0)


/*
	size_t

	Note: this definition is implementation-dependent and therefore shouldn't really be here. As it
	is envisaged that this library will only ever be used on MC680x0 systems, size_t is hard-wired
	to a 32-bit value here.
*/
#ifndef HAVE_SIZE_T
#define HAVE_SIZE_T
typedef unsigned int size_t;
#endif


/*
	ptrdiff_t

	Note: see the notes for the size_t definition.
*/

#ifndef HAVE_PTRDIFF_T
#define HAVE_PTRDIFF_T
typedef signed int ptrdiff_t;
#endif


/*
	offsetof
*/

#ifndef offsetof
#define offsetof(s, m) ((size_t) ((char *)&((s *) NULL)->m - (char *) NULL))
#endif

/*
	wchar_t
*/

#ifndef HAVE_WCHAR_T
#define HAVE_WCHAR_T
typedef unsigned int wchar_t;
#endif


#endif

