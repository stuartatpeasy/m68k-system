#ifndef KUTIL_KUTIL_H_INC
#define KUTIL_KUTIL_H_INC
/*
	kutil.h - declarations of the k*() utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2012-07
*/

#include <stdarg.h>
#include "include/error.h"
#include "include/types.h"
#include "klibc/string.h"

/*
    Checksum functions
*/

u16 fletcher16(const void *buf, u32 len);

#define CHECKSUM16(buf, len)    fletcher16((buf), (len))


/*
    Date/time-related functions
*/
s32 date_iso8601(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 date_short(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 date_long(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 time_iso8601(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 rtc_time_from_str(const char * const str, struct rtc_time * const tm);
ks8 * const day_number_suffix(ku8 daynum);
s32 day_of_week(ks32 year, ks32 month, ks32 day);
s32 is_leap_year(ks32 year);
s32 timestamp_to_rtc_time(ks32 timestamp, struct rtc_time *dt);
s32 rtc_time_to_timestamp(const struct rtc_time *dt, s32 *timestamp);


#define VALID_RTC_DATE(d)                                   \
    (((d)->year < 2200) && ((d)->year >= 1900)              \
       && ((d)->month > 0) && ((d)->month < 13)             \
       && ((d)->day > 0) && ((d)->day < 32)                 \
       && ((d)->hour < 24)                                  \
       && ((d)->minute < 60)                                \
       && ((d)->second < 60)                                \
       && ((d)->day_of_week > 0) && ((d)->day_of_week < 8))

/*
    String-related functions
*/
s32 dump_hex(void *p, ku32 word_size, ku32 offset, ku32 num_bytes);
ks8 *kstrerror(ks32 errnum);
s8 *str_trim(s8 *dest, ks8 *src);

#endif

