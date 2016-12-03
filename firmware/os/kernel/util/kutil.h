#ifndef KERNEL_UTIL_KUTIL_H_INC
#define KERNEL_UTIL_KUTIL_H_INC
/*
	kutil.h - declarations of the k*() utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2012-07
*/

#include <stdarg.h>
#include <kernel/include/types.h>
#include <kernel/include/error.h>
#include <klibc/include/string.h>


/*
    Checksum functions
*/

u16 fletcher16(const void *buf, u32 len);

#define CHECKSUM16(buf, len)    fletcher16((buf), (len))


/*
    Date/time-related functions
*/
s32     date_iso8601(const rtc_time_t * const tm, char * const buffer, ku32 len);
s32     date_short(const rtc_time_t * const tm, char * const buffer, ku32 len);
s32     date_long(const rtc_time_t * const tm, char * const buffer, ku32 len);
ks8 *   day_number_suffix(ku8 daynum);
s32     day_of_week(ks32 year, ks32 month, ks32 day);
s32     get_time(rtc_time_t *tm);
s32     is_leap_year(ks32 year);
s32     rtc_time_from_str(const char * const str, rtc_time_t * const tm);
s32     rtc_time_to_timestamp(const rtc_time_t *dt, time_t *timestamp);
s32     time_iso8601(const rtc_time_t * const tm, char * const buffer, ku32 len);
s32     timestamp_to_rtc_time(const time_t timestamp, rtc_time_t *dt);


#define VALID_RTC_DATE(d)                                   \
    (((d)->year < 2200) && ((d)->year >= 1900)              \
       && ((d)->month > 0) && ((d)->month < 13)             \
       && ((d)->day > 0) && ((d)->day < 32)                 \
       && ((d)->hour < 24)                                  \
       && ((d)->minute < 60)                                \
       && ((d)->second < 60)                                \
       && ((d)->day_of_week > 0) && ((d)->day_of_week < 8))

/*
	Hashing functions
*/
u32 fnv1a32(const void *buf, u32 len);


/*
    Numeric functions
*/
u16 log10(ku32 n);
u16 log2(u32 n);


/*
    String-related functions
*/
s32     dump_hex(const void *p, ku32 word_size, ku32 offset, ku32 num_bytes);
ks8 *   kstrerror(ks32 errnum);
u32		str_sum(ks8 *s);
s8 *    str_trim(s8 *dest, ks8 *src);
s8 *    strn_trim(s8 *dest, ks8 *src, u32 n);
s8 *    strn_trim_cpy(s8 *dest, s8 *src, ku32 len);

#endif

