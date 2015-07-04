#ifndef __INCLUDE_TYPES_H__
#define __INCLUDE_TYPES_H__
/*
	Type definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.
*/

typedef unsigned char				u8;
typedef const unsigned char			ku8;
typedef volatile unsigned char		vu8;
typedef signed char					s8;
typedef const signed char			ks8;
typedef volatile signed char		vs8;

typedef unsigned short int			u16;
typedef const unsigned short int	ku16;
typedef volatile unsigned short int	vu16;
typedef signed short int			s16;
typedef const signed short int		ks16;
typedef volatile signed short int	vs16;

typedef unsigned int				u32;
typedef const unsigned int			ku32;
typedef volatile unsigned int		vu32;
typedef signed int					s32;
typedef const signed int			ks32;
typedef volatile signed int			vs32;

typedef unsigned long long			u64;
typedef const unsigned long long	ku64;
typedef volatile unsigned long long	vu64;
typedef signed long long			s64;
typedef const signed long long		ks64;
typedef volatile signed long long	vs64;

/* Wall-clock time */
struct rtc_time
{
    u16     year;
    u8      month;
    u8      day;
    u8      hour;
    u8      minute;
    u8      second;
    u8      day_of_week;
    u8      dst;
};

/* This probably isn't the right place to define NULL, but I don't think it warrants its
   own header file. */
#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif

