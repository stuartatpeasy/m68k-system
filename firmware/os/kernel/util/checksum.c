/*
    Checksum-generation functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/util/kutil.h>


/*
    fletcher16() - compute the 16-bit Fletcher checksum of len bytes at buf.
    See: https://en.wikipedia.org/wiki/Fletcher%27s_checksum
*/
u16 fletcher16(const void *buf, u32 len)
{
    u16 sum1 = 0xff, sum2 = 0xff;

    while(len)
    {
        u32 tlen = (len > 20) ? 20 : len;
        len -= tlen;

        do
        {
            sum2 += sum1 += *((u8 *) buf++);
        } while(--tlen);

        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }

    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);

    return (sum2 << 8) | sum1;
}
