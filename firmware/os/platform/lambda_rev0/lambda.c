/*
    ayumos port for the "lambda" rev0 (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/lambda_rev0/lambda.h>
#include <platform/lambda_rev0/device.h>
#include <device/device.h>
#include <device/ds17485.h>


mem_extent_t g_lambda_mem_extents[] =
{
    {
        .base   = (void *) 0x00000000,
        .len    = 256 * 1024,
        .flags  = MEM_EXTENT_KERN | MEM_EXTENT_RAM
    },

    {
        .base   = (void *) 0x00040000,
        .len    = 0,                    /* will be filled in during RAM detection */
        .flags  = MEM_EXTENT_USER | MEM_EXTENT_RAM
    }
};


s32 plat_init(void)
{
    /* Nothing to do here */
    return SUCCESS;
}


s32 plat_mem_detect()
{
    /*
        The system supports between 1MB and 8MB of RAM, in units of 1MB.  At least 1MB must be
        installed, or we would not have reached this point.

        Detect RAM by writing a test value at 1MB intervals, then reading it back.  Store in
        g_mem_top the address of the byte after the last byte of RAM.
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

    g_lambda_mem_extents[1].len = ((u32) p) - ((u32) g_lambda_mem_extents[1].base);

    g_mem_extents = g_lambda_mem_extents;
    g_mem_extents_end = &g_lambda_mem_extents[2];   /* ptr to first non-existent extent */

    return SUCCESS;
}


/*
    plat_console_init() - initialise the boot console.  This happens early in the boot process -
    before the main hardware-enumeration step.  This code therefore builds two dev_t objects in
    global variables (g_lambda_duart, g_lambda_console); these are later added to the device tree in
    plat_dev_enumerate().
*/
s32 plat_console_init(void)
{
    s32 ret;

    /* Initialise the console */
    ret = dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", 0, (void *) 0xe00000,
                        &g_lambda_duart);
    if(ret != SUCCESS)
        return ret;

    ret = mc68681_init(g_lambda_duart);
    if(ret != SUCCESS)
    {
        kfree(g_lambda_duart);
        g_lambda_duart = NULL;
        return ret;
    }

	ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000,
                        &g_lambda_console);
	if(ret == SUCCESS)
	{
	    g_lambda_console->parent = g_lambda_duart;
		ret = mc68681_serial_a_init(g_lambda_console);
		if(ret != SUCCESS)
		{
			kfree(g_lambda_console);
			g_lambda_console = NULL;
		}
	}

	return ret;
}


/*
    plat_console_putc() - write a character to the console.
*/
s16 plat_console_putc(const char c)
{
    return mc68681_channel_a_putc(g_lambda_console, c);
}


/*
    plat_console_getc() - read a character from the console.
*/
s16 plat_console_getc()
{
    return mc68681_channel_a_getc(g_lambda_console);
}


/*
    plat_start_quantum() - start the quantum timer, i.e. begin a new process time-slice.

    Note: this function will be called in interrupt context.
*/
s32 plat_start_quantum()
{
    mc68681_start_counter(g_lambda_console, (MC68681_CLK_HZ / 16) / TICK_RATE);
    return SUCCESS;
}


/*
    plat_stop_quantum() - take any action necessary to finish the previous process time-slice.

    Note: this function will be called in interrupt context.
*/
s32 plat_stop_quantum()
{
    mc68681_stop_counter(g_lambda_console);
    return SUCCESS;
}


/*
    plat_led_on() - switch on one or more of the motherboard LEDs.
*/
s32 plat_led_on(ku8 leds)
{
    mc68681_reset_op_bits(g_lambda_console, leds & (LED_RED | LED_GREEN));
    return SUCCESS;
}


/*
    plat_led_off() - switch off one or more of the motherboard LEDs.
*/
s32 plat_led_off(ku8 leds)
{
    mc68681_set_op_bits(g_lambda_console, leds & (LED_RED | LED_GREEN));
    return SUCCESS;
}


/*
    plat_get_serial_number() - write a unique serial number into sn[8].
*/
s32 plat_get_serial_number(u8 sn[8])
{
    dev_t *dev = dev_find("rtc0");
    if(dev)
    {
        sn[0] = 0;
        sn[1] = 0;
        ds17485_get_serial_number(dev, sn + 2);

        return SUCCESS;
    }

    return ENOSYS;
}
