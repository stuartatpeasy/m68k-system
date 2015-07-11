/*
    RAM detection algorithm

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <memory/ramdetect.h>

u32 g_ram_top = 0;


u32 ram_detect(void)
{
    /*
        The system supports between 1MB and 8MB of RAM, in units of 1MB.  At least 1MB must be
        installed, or we would not have reached this point.

        Detect RAM by writing a test value at 1MB intervals, then reading it back.  Store in
        g_ram_top the address of the byte after the last byte of RAM, and return this value.
    */

    ku32 test_val = 0x08080808;

    vu32 *p;
    u32 val = test_val;
    for(p = (u32 *) 0x100000; p < (u32 *) USER_RAM_END; p += 0x40000, val += 0x11111111)
    {
        *p = val;
        if(*p != val)
            break;
    }

    g_ram_top = (u32) p;

    return g_ram_top;
}
