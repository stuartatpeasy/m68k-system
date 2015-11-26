/*
    Miscellaneous numeric helper functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, September 2015
*/

#include <kernel/util/kutil.h>


/*
    log10() - return the integer log to base 10 of n.

    NOTE: log10(0) returns 0!
*/
u16 log10(ku32 n)
{
    if     (n < 10)         return 0;       /* NOTE: we define log10(0) == 0 */
    else if(n < 100)        return 1;
    else if(n < 1000)       return 2;
    else if(n < 10000)      return 3;
    else if(n < 100000)     return 4;
    else if(n < 1000000)    return 5;
    else if(n < 10000000)   return 6;
    else if(n < 100000000)  return 7;
    else if(n < 1000000000) return 8;
    else                    return 9;
}


/*
    log2() - return the integer log to base 2 of n.

    NOTE: log2(0) returns 0!
*/
u16 log2(u32 n)
{
    u16 ret = 0;

    if(n >= 0x00010000)
    {
        ret = 16;
        n >>= 16;
    }

    if(n >= 0x0100)
    {
        ret += 8;
        n >>= 8;
    }

    if(n >= 0x10)
    {
        ret += 4;
        n >>= 4;
    }

    if(n >= 0x04)
    {
        ret += 2;
        n >>= 2;
    }

    return (n >= 2) ? ret + 1 : ret;
}
