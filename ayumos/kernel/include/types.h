#ifndef KERNEL_INCLUDE_TYPES_H_INC
#define KERNEL_INCLUDE_TYPES_H_INC
/*
	Type definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.
*/


/*
	Basic types - these should not be used in code.  They can be overridden for specific
	host system architectures.
*/
#define T_S8				char
#define T_U8				unsigned char
#define T_S16				signed short int
#define T_U16				unsigned short int
#define T_S32				signed int
#define T_U32				unsigned int
#define T_S64				signed long long int
#define T_U64				unsigned long long int


/* Size-specific typedefs, to be used in kernel code */
typedef T_U8				u8;
typedef const T_U8			ku8;
typedef volatile T_U8		vu8;
typedef T_S8				s8;
typedef const T_S8			ks8;
typedef volatile T_S8		vs8;

typedef T_U16				u16;
typedef const T_U16			ku16;
typedef volatile T_U16		vu16;
typedef T_S16				s16;
typedef const T_S16			ks16;
typedef volatile T_S16		vs16;

typedef T_U32				u32;
typedef const T_U32			ku32;
typedef volatile T_U32		vu32;
typedef T_S32				s32;
typedef const T_S32			ks32;
typedef volatile T_S32		vs32;

typedef T_U64				u64;
typedef const T_U64			ku64;
typedef volatile T_U64		vu64;
typedef T_S64				s64;
typedef const T_S64			ks64;
typedef volatile T_S64		vs64;

/* Size type */
#ifndef HAVE_SIZE_T
typedef u32 size_t;
#endif

/* Memory address type */
typedef u32 addr_t;

/* Wall-clock time */
typedef struct rtc_time
{
    u16     year;
    u8      month;          /* 1=Jan, ..., 12=Dec */
    u8      day;
    u8      hour;
    u8      minute;
    u8      second;
    u8      day_of_week;
    u8      dst;
} rtc_time_t;

/* Process ID */
/* Test harnesses may already have defined pid_t, so define it conditionally here */
#ifndef HAVE_PID_T
typedef s16 pid_t;
#endif

/* Process entry-point function */
typedef void (*proc_entry_fn_t)(void *);


/* In-memory executable image */
struct exe_img
{
    void *start;                    /* Ptr to first byte of img in memory   */
    u32 len;                        /* Img len                              */
    proc_entry_fn_t entry_point;    /* Ptr to first instruction of img      */
};

typedef struct exe_img exe_img_t;

/* CPU registers */
typedef u16 reg16_t;
typedef u32 reg32_t;
typedef u64 reg64_t;

/* Time-related definitions */
typedef s32 time_t;

/* This probably isn't the right place to define NULL, but I don't think it warrants its
   own header file. */
#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif

