#ifndef __INCLUDE_KUTIL_H__
#define __INCLUDE_KUTIL_H__
/*
	kutil.h - declarations of the k*() utility functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2012-07
*/

#include <stdarg.h>
#include "include/types.h"


s32 date_iso8601(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 date_short(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 date_long(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 time_iso8601(const struct rtc_time * const tm, char * const buffer, ku32 len);
s32 rtc_time_from_str(const char * const str, struct rtc_time * const tm);
ks8 * const day_number_suffix(ku8 daynum);
s32 day_of_week(ks32 year, ks32 month, ks32 day);
s32 is_leap_year(ks32 year);
s32 timestamp_to_rtc_time(ks32 timestamp, struct rtc_time *dt);

void kbzero(void *s, u32 n);
void *kmemcpy(void *dest, const void *src, ku32 n);
void kprintf(ks8 *format, ...);
s32 kputchar(s32 c);
void kput(ks8 *s);
void kputs(ks8 *s);
s8 *kstrcat(s8 *dest, ks8 *src);
s32 kstrcmp(ks8 *s1, ks8 *s2);
s8 *kstrcpy(s8 *dest, ks8 *src);
s8 *kstrdup(ks8 *s);
u32 kstrlen(ks8 *s);
s32 kstrncmp(ks8 *s1, ks8 *s2, u32 n);
s8 *kstrncpy(s8 *dest, ks8 *src, u32 n);
s8 *kstrstr(ks8 *haystack, ks8 *needle);
u32 kstrtoul(ks8 *nptr, s8 **endptr, s32 base);

s32 printf(const char *format, ...);
s32 snprintf(char *str, u32 size, const char *format, ...);
s32 sprintf(char *str, const char *format, ...);
s32 vprintf(const char *format, va_list ap);
s32 vsprintf(char *str, const char *format, va_list ap);
s32 vsnprintf(char *str, u32 size, const char *format, va_list ap);


#define VALID_RTC_DATE(d)                                   \
    (((d)->year < 2200) && ((d)->year >= 1900)              \
       && ((d)->month > 0) && ((d)->month < 13)             \
       && ((d)->day > 0) && ((d)->day < 32)                 \
       && ((d)->hour < 24)                                  \
       && ((d)->minute < 60)                                \
       && ((d)->second < 60)                                \
       && ((d)->day_of_week > 0) && ((d)->day_of_week < 8))

#endif

