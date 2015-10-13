/*
    ktime.c: functions for working with RTC time structs

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include "include/defs.h"
#include "include/limits.h"
#include "include/types.h"
#include "klibc/stdio.h"
#include "kutil.h"

#define DATETIME_TS_MAX (S32_MAX)

const char * const g_day_names_long[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

const char * const g_day_names_short[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char * const g_month_names_long[] =
{
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

const char * const g_month_names_short[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* Offsets, in seconds, of the start of each month from the start of the year in common years.
   The first entry represents the offset of 1st February 00:00:00. */
static ks32 g_ts_month_offset_common[] =
{
     2678400,  5097600,  7776000, 10368000, 13046400, 15638400,
    18316800, 20995200, 23587200, 26265600, 28857600,  DATETIME_TS_MAX
};

/* Offsets, in seconds, of the start of each month from the start of the year in leap years.
   The first entry represents the offset of 1st February 00:00:00. */
static ks32 g_ts_month_offset_leap[] =
{
     2678400,  5184000,  7862400, 10454400, 13132800, 15724800,
    18403200, 21081600, 23673600, 26352000, 28944000,  DATETIME_TS_MAX
};

/* Timestamps of the first second in each year from 1901 - 2037 */
static ks32 g_ts_year_offset[] =
{
    -2147483648, -2145916800, -2114380800, -2082844800,
    -2051222400, -2019686400, -1988150400, -1956614400,
    -1924992000, -1893456000, -1861920000, -1830384000,
    -1798761600, -1767225600, -1735689600, -1704153600,
    -1672531200, -1640995200, -1609459200, -1577923200,
    -1546300800, -1514764800, -1483228800, -1451692800,
    -1420070400, -1388534400, -1356998400, -1325462400,
    -1293840000, -1262304000, -1230768000, -1199232000,
    -1167609600, -1136073600, -1104537600, -1073001600,
    -1041379200, -1009843200,  -978307200,  -946771200,
     -915148800,  -883612800,  -852076800,  -820540800,
     -788918400,  -757382400,  -725846400,  -694310400,
     -662688000,  -631152000,  -599616000,  -568080000,
     -536457600,  -504921600,  -473385600,  -441849600,
     -410227200,  -378691200,  -347155200,  -315619200,
     -283996800,  -252460800,  -220924800,  -189388800,
     -157766400,  -126230400,   -94694400,   -63158400,
      -31536000,           0,    31536000,    63072000,
       94694400,   126230400,   157766400,   189302400,
      220924800,   252460800,   283996800,   315532800,
      347155200,   378691200,   410227200,   441763200,
      473385600,   504921600,   536457600,   567993600,
      599616000,   631152000,   662688000,   694224000,
      725846400,   757382400,   788918400,   820454400,
      852076800,   883612800,   915148800,   946684800,
      978307200,  1009843200,  1041379200,  1072915200,
     1104537600,  1136073600,  1167609600,  1199145600,
     1230768000,  1262304000,  1293840000,  1325376000,
     1356998400,  1388534400,  1420070400,  1451606400,
     1483228800,  1514764800,  1546300800,  1577836800,
     1609459200,  1640995200,  1672531200,  1704067200,
     1735689600,  1767225600,  1798761600,  1830297600,
     1861920000,  1893456000,  1924992000,  1956528000,
     1988150400,  2019686400,  2051222400,  2082758400,
     2114380800,  2145916800
};

/* Magic numbers used in day-of-week calculation algorithm for common years: the index into this
   array is (year_of_century % 28). */
static ks8 g_dow_common_year_table[] =
{
    0, 1, 2, 3, 5, 6, 0,
    1, 3, 4, 5, 6, 1, 2,
    3, 4, 6, 0, 1, 2, 4,
    5, 6, 0, 2, 3, 4, 5
};

/* Magic numbers used in day-of-week calculation algorithm for leap years: the index into this
   array is (year_of_century % 28) / 4. */
static ks8 g_dow_leap_year_table[] =
{
    6, 4, 2, 0, 5, 3, 1
};

/* Magic numbers used in day-of-week calculation: the index into this array is the zero-based
   month number. */
static ks8 g_dow_month_table[] =
{
    0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5
};

/* Number of days in each month in a leap year */
static ks8 g_days_in_month_leap[] =
{
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


/*
    date_iso8601() - write the date and time, in ISO-8601, to buffer.  E.g.
    "2017-01-12 14:25:36"
*/
s32 date_iso8601(const rtc_time_t * const tm, char * const buffer, ku32 len)
{
    if(!VALID_RTC_DATE(tm))
        return FAIL;

    return snprintf(buffer, len, "%04u-%02u-%02u %02u:%02u:%02u",
                        tm->year, tm->month, tm->day,
                        tm->hour, tm->minute, tm->second);
}


/*
    date_short() - write the date, specified in tm, to buffer in "short" format,
    e.g. "Mon 12 Jan 2017".
*/
s32 date_short(const rtc_time_t * const tm, char * const buffer, ku32 len)
{
    if(!VALID_RTC_DATE(tm))
        return FAIL;

    return snprintf(buffer, len, "%s %u %s %u",
                        g_day_names_short[tm->day_of_week - 1], tm->day,
                        g_month_names_short[tm->month - 1], tm->year);
}


/*
    date_long() - write the date, specified in tm, to buffer in "long" format,
    e.g. "Monday 12th January 2017".
*/
s32 date_long(const rtc_time_t * const tm, char * const buffer, ku32 len)
{
    if(!VALID_RTC_DATE(tm))
        return FAIL;

    return snprintf(buffer, len, "%s %u%s %s %u",
                        g_day_names_long[tm->day_of_week - 1], tm->day,
                        day_number_suffix(tm->day),
                        g_month_names_long[tm->month - 1], tm->year);
}


/*
    time_iso8601() - write the time, specified in tm, to buffer in ISO-8601 format,
    e.g. "14:25:36".
*/
s32 time_iso8601(const rtc_time_t * const tm, char * const buffer, ku32 len)
{
    if(!VALID_RTC_DATE(tm))
        return FAIL;

    return snprintf(buffer, len, "%02u:%02u:%02u",
                        tm->hour, tm->minute, tm->second);
}


/*
    rtc_time_from_str() - take a string, in the format "YYYYMMDDHHMMSS", and populate
    tm with the values.
*/
s32 rtc_time_from_str(const char * const str, rtc_time_t * const tm)
{
    s32 i;
    s8 s[14];

    // Set the date and time.  Acceptable format:
    //      date YYYYMMDDHHMMSS
    if(strlen(str) != 14)
        return FAIL;

    for(i = 0; i < 14; i++)
    {
        if((str[i] >= '0') && (str[i] <= '9'))
            s[i] = str[i] - '0';
        else
            return FAIL;
    }

    tm->year    = (s[0] * 1000) + (s[1] * 100) + (s[2] * 10) + s[3];
    tm->month   = (s[4] * 10) + s[5];
    tm->day     = (s[6] * 10) + s[7];
    tm->hour    = (s[8] * 10) + s[9];
    tm->minute  = (s[10] * 10) + s[11];
    tm->second  = (s[12] * 10) + s[13];

    // Day of week
    tm->day_of_week = day_of_week(tm->year, tm->month, tm->day);

    return VALID_RTC_DATE(tm) ? SUCCESS : FAIL;
}


/*
    day_number_suffix() - return a pointer to a string containing an appropriate suffix
    for a day number.  E.g. for daynum = 21, the returned suffix string will be "st".
*/
ks8 * day_number_suffix(const u8 daynum)
{
    if((daynum < 1) || (daynum > 31))
        return "";

    switch(daynum)
    {
        case 1:
        case 21:
        case 31:
            return "st";

        case 2:
        case 22:
            return "nd";

        case 3:
        case 23:
            return "rd";

        default:
            return "th";
    }
}


/* Calculate the day of week given a year, month and day.  This function must only be called with
   a year/month/day combination in the range 1900-01-01 to 2099-12-31; otherwise, behaviour is
   undefined. It returns a value in the range 0 (Sunday) to 6 (Saturday). */
static s32 day_of_week_unchecked(ks32 year, ks32 month, ks32 day)
{
    s32 a = (year >= 2000) ? 6 : 0;
    s32 b = (year >= 2000) ? year - 2000 : year - 1900;

    if(b >= 56) b -= 56;
    if(b >= 28) b -= 28;

    if((year != 1900) && !(year & 3) && (month < 3))    /* Mar-Dec in leap year? */
        a += g_dow_leap_year_table[b >> 2];
    else
        a += g_dow_common_year_table[b];

    a += g_dow_month_table[month - 1] + day;

    if(a >= 28) a -= 28;
    if(a >= 14) a -= 14;
    if(a >= 7) a -= 7;

    return a + 1;   /* 1 = Sunday, 2 = Monday, ..., 7 = Saturday */
}


/* Calculate the day of week for a given year, month and day.  This function returns a value in
   the range 0 (Sunday) to 6 (Saturday) in the case of valid dates, and -1 otherwise.  Valid dates
   are those in the range 1900-01-01 to 2099-12-31. */
s32 day_of_week(ks32 year, ks32 month, ks32 day)
{
    if((year < 1900) || (year > 2099) || (month < 1) || (month > 12) || (day < 1) ||
       (day > g_days_in_month_leap[month - 1]) ||
       ((month == 2) && (day == 29) && !is_leap_year(year)))
        return -1;

    return day_of_week_unchecked(year, month, day);
}


/* Determine whether a year is a leap or common year.  Returns non-zero in the case of a leap year
   and zero for a common year.  Considers (the nonexistent) year 0 to be a leap year. */
s32 is_leap_year(ks32 year)
{
    return !((year & 3) || (!(year % 100) && (year % 400)));
}


/* Convert a Unix timestamp to a rtc_time_t */
s32 timestamp_to_rtc_time(ks32 timestamp, rtc_time_t *dt)
{
    s32 year_index, step, t_;
    ks32 *month_offset;

    /* -- Year -- */
    /* Unix time spans 138 years from 1901-2038.  A lookup table, g_ts_year_offset[], contains
       the timestamp values of the first second of each year (except 1901, which is represented
       by INT_MIN).  To find the year, a binary search with an initial step size of 32 is used.
       Starting at 1970, this gives a range from 1907 to 2033.  If the timestamp falls outside
       this range, the algorithm falls back to a linear search to finish resolving the year. */
    for(year_index = 69, step = 32; step; step >>= 1)
    {
        if(g_ts_year_offset[year_index] > timestamp)
            year_index -= step;
        else if(g_ts_year_offset[year_index + 1] <= timestamp)
            year_index += step;
        else break;
    }

    /* These two while() statements resolve years earlier than 1907 and later than 2033 */
    while((g_ts_year_offset[year_index] > timestamp) && year_index)
        --year_index;

    while((timestamp >= g_ts_year_offset[year_index + 1]) &&
            (year_index < ((sizeof(g_ts_year_offset) / sizeof(g_ts_year_offset[0])) - 1)))
        ++year_index;

    dt->year = year_index + 1901;

    /* -- Month -- */
    t_ = timestamp - g_ts_year_offset[year_index];
    if(!year_index)
    {
        /* The timestamp of the first second supported by Unix time, -2147483648, represents
           Fri 13 Dec 1901, 20:45:02.  All subsequent calculations require that t_ is an
           offset into a year (i.e. t_=0 means "00:00:00 on 1st January").  For all years
           after 1901, the g_ts_year_offset[] table contains an entry representing this
           second; in the case of 1901, the entry represents to Fri 13 Dec 20:45:02.  To
           avoid integer overflow, the offset must be fixed up here. */

        t_ += 29969152; /* # secs in common year between 1 Jan 00:00:00 -> 13 Dec 20:45:02 */
    }

    /* g_ts_month_offset_common[] and g_ts_month_offset_leap[] are lookup tables containing the
       offset of the first second in each month from the start of the year in common and leap
       years respectively. */
    month_offset = ((year_index + 1) & 3) ? g_ts_month_offset_common : g_ts_month_offset_leap;

    for(dt->month = 0; month_offset[dt->month] <= t_; ++dt->month) ;

    if(dt->month)
        t_ -= month_offset[dt->month - 1];

    /* Day */
    for(dt->day = 1, step = 4; step >= 0; --step)
        if(t_ >= (86400 << step))
        {
            t_ -= 86400 << step;
            dt->day += 1 << step;
        }

    /* Day of week */
    dt->day_of_week = day_of_week_unchecked(dt->year, dt->month + 1, dt->day);

    /* Hour */
    for(dt->hour = 0, step = 4; step >= 0; --step)
        if(t_ >= (3600 << step))
        {
            t_ -= 3600 << step;
            dt->hour += 1 << step;
        }

    /* Minute */
    for(dt->minute = 0, step = 5; step >= 0; --step)
        if(t_ >= (60 << step))
        {
            t_ -= 60 << step;
            dt->minute += 1 << step;
        }

    /* Second */
    dt->second = t_;

    return SUCCESS;
}


/* Convert a rtc_time_t to a Unix timestamp */
s32 rtc_time_to_timestamp(const rtc_time_t *dt, s32 *timestamp)
{
    s32 ts;

    /*
        Earliest date representable as a timestamp: 1901-12-13 20:45:52
        (i.e. year=1901, month=12, day=13, hour=20, minute=45, second=52)
    */
    if((dt->year <= 1901) && (dt->month <= 12) && (dt->day <= 13) &&
       (dt->hour <= 20) && (dt->minute <= 45) && (dt->second <= 52))
        return EINVAL;

    /*
        Latest date representable as a timestamp: 2038-01-19 03:14:07
        (i.e. year=2038, month=1, day=19, hour=3, minute=14, second=7)
    */
    if((dt->year >= 2038) && (dt->month >= 1) && (dt->day >= 19) &&
       (dt->hour >= 3) && (dt->minute >= 14) && (dt->second >= 7))
        return EINVAL;

    ts = g_ts_year_offset[dt->year - 1901];

    /* If the month in *dt is not January, add an offset representing the start of the month */
    if(dt->month > 1)
    {
        ts += (is_leap_year(dt->year)) ?
            g_ts_month_offset_leap[dt->month - 2] : g_ts_month_offset_common[dt->month - 2];
    }

    ts += (86400 * (dt->day - 1))
            + (3600 * dt->hour)
            + (60 * dt->minute)
            + dt->second;

    *timestamp = ts;

    return SUCCESS;
}


/*
    get_time() - read the current wall-clock time from the first RTC in the system.
*/
s32 get_time(rtc_time_t *tm)
{
    dev_t *rtc;

    rtc = dev_find("rtc0");
    if(rtc == NULL)
        return ENOSYS;

    return ((rtc_ops_t *) rtc->driver)->get_time(rtc, tm);
}
