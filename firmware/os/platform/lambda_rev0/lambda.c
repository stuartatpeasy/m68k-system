/*
    ayumos port for the "lambda" rev0 (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/lambda_rev0/lambda.h>


u32 g_nram_extents = 2;
ram_extent_t g_ram_extents[] =
{
    {
        .base   = (void *) 0x00000000,
        .len    = 256 * 1024,
        .flags  = RAM_EXTENT_KERN
    },

    {
        .base   = (void *) 0x00040000,
        .len    = 0,                    /* will be filled in during RAM detection */
        .flags  = RAM_EXTENT_KERN
    }
};


void plat_cpu_init(void)
{
    /* Nothing to do here */
}


void plat_ram_init(void)
{
    /* Nothing to do here */
}


void plat_ram_detect()
{
    /*
        The system supports between 1MB and 8MB of RAM, in units of 1MB.  At least 1MB must be
        installed, or we would not have reached this point.

        Detect RAM by writing a test value at 1MB intervals, then reading it back.  Store in
        g_ram_top the address of the byte after the last byte of RAM.
    */

    ku32 test_val = 0x08080808;
    vu32 *p;
    u32 val = test_val;

    for(p = (u32 *) 0x100000; p < (u32 *) 0x00800000; p += 0x40000, val += 0x11111111)
    {
        *p = val;
        if(*p != val)
            break;
    }

    g_ram_extents[1].len = ((u32) p) - ((u32) g_ram_extents[1].base);
}


void plat_console_init(void)
{
    /* Initialise the console */
}
