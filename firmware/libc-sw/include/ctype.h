#ifndef _LIBC_STUARTW_CTYPE_H_
#define _LIBC_STUARTW_CTYPE_H_
/*
	ctype.h

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07-01.
*/

#define _ISUPPER	(0x0001)
#define _ISLOWER	(0x0002)
#define _ISALPHA	(0x0004)
#define _ISDIGIT	(0x0008)
#define _ISXDIGIT	(0x0010)
#define _ISSPACE	(0x0020)
#define _ISPRINT	(0x0040)
#define _ISGRAPH	(0x0080)
#define _ISBLANK	(0x0100)
#define _ISCNTRL	(0x0200)
#define _ISPUNCT	(0x0400)
#define _ISALNUM	(0x0800)


#define isupper(c)	_ctypeisname(c, _ISUPPER)
#define islower(c)	_ctypeisname(c, _ISLOWER)
#define isalpha(c)	_ctypeisname(c, _ISALPHA)
#define isdigit(c)	_ctypeisname(c, _ISDIGIT)
#define isxdigit(c)	_ctypeisname(c, _ISXDIGIT)
#define isspace(c)	_ctypeisname(c, _ISSPACE)
#define isprint(c)	_ctypeisname(c, _ISPRINT)
#define isgraph(c)	_ctypeisname(c, _ISGRAPH)
#define isblank(c)	_ctypeisname(c, _ISBLANK)
#define iscntrl(c)	_ctypeisname(c, _ISCNTRL)
#define ispunct(c)	_ctypeisname(c, _ISPUNCT)
#define isalnum(c)	_ctypeisname(c, _ISALNUM)

#define isascii(c)	(((c) & ~0x7f) == 0)
#define toascii(c)	((c) & 0x7f)

int tolower(int c);
int toupper(int c);

inline int _ctypeisname(int c, const unsigned short int type);

#endif

