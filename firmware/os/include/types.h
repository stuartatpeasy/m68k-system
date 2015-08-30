#ifndef INCLUDE_TYPES_H_INC
#define INCLUDE_TYPES_H_INC
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

/* Test harnesses may already have defined size_t, so define it conditionally here */
#ifndef HAVE_SIZE_T
typedef u32 size_t;
#endif

/* Wall-clock time */
struct rtc_time
{
    u16     year;
    u8      month;          /* 1=Jan, ..., 12=Dec */
    u8      day;
    u8      hour;
    u8      minute;
    u8      second;
    u8      day_of_week;
    u8      dst;
};

typedef struct rtc_time rtc_time_t;

/* Process ID */
/* Test harnesses may already have defined pid_t, so define it conditionally here */
#ifndef HAVE_PID_T
typedef s16 pid_t;
#endif

/* In-memory executable image */
struct exe_img
{
    void *start;            /* Ptr to first byte of img in memory   */
    u32 len;                /* Img len                              */
    u16 *entry_point;       /* Ptr to first instruction of img      */
};

typedef struct exe_img exe_img_t;


/* This probably isn't the right place to define NULL, but I don't think it warrants its
   own header file. */
#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif

