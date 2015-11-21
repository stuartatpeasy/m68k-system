/*
    ayumos port for the "lambda" rev0 (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/platform.h>
#include <platform/lambda_rev0/lambda.h>
#include <platform/lambda_rev0/device.h>
#include <kernel/device/device.h>
#include <device/ds17485.h>
#include <device/mc68681.h>


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
    },
    {
        .base   = (void *) 0x00800000,
        .len    = 0x00200000,
        .flags  = MEM_EXTENT_VACANT
    },
    {
        .base   = (void *) 0x00a00000,
        .len    = 0x00500000,
        .flags  = MEM_EXTENT_PERIPH
    },
    {
        .base   = (void *) 0x00f00000,
        .len    = 0x00100000,
        .flags  = MEM_EXTENT_KERN | MEM_EXTENT_ROM
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
    g_mem_extents_end = &g_lambda_mem_extents[ARRAY_COUNT(g_lambda_mem_extents)];

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
    ret = dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", "MC68681 DUART",
                     LAMBDA_MC68681_IRQL, LAMBDA_MC68681_BASE, &g_lambda_duart);
    if(ret != SUCCESS)
        return ret;

    ret = mc68681_init(g_lambda_duart);
    if(ret != SUCCESS)
    {
        kfree(g_lambda_duart);
        g_lambda_duart = NULL;
        return ret;
    }

	ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", "MC68681 serial port A",
                        LAMBDA_MC68681_IRQL, LAMBDA_MC68681_BASE, &g_lambda_console);
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
    mc68681_channel_a_putc(g_lambda_console, c);
    return c;
}


/*
    plat_console_getc() - read a character from the console.
*/
s16 plat_console_getc()
{
    char c;
    mc68681_channel_a_getc(g_lambda_console, &c);

    return c;
}


/*
	plat_install_timer_irq_handler() - bind the timer IRQ to the appropriate handler in the OS.
*/
s32 plat_install_timer_irq_handler(irq_handler handler)
{
    ks32 ret = mc68681_set_output_pin_fn(g_lambda_duart, mc68681_pin_op3, mc68681_pin_fn_ct_output);

    if(ret != SUCCESS)
        return ret;

    CPU_EXC_VPTR_SET(V_level_1_autovector, handler);

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


/*
    plat_get_cpu_clock() - estimate the CPU clock frequency in Hz
*/
s32 plat_get_cpu_clock(u32 *clk)
{
    /* Ensure that interrupts are disabled before entering this section */
    rtc_time_t tm;
    u32 loops;
    u8 curr_second;
    dev_t *rtc;
    s32 (*rtc_get_time)(dev_t *, ku32 offset, u32 *len, void *);
    u32 one = 1;
    s32 ret;

    /* Find the first RTC */
    rtc = dev_find("rtc0");
    if(rtc == NULL)
        return ENOSYS;

    rtc_get_time = rtc->read;

    /* Wait for the next second to start */
    ret = rtc_get_time(rtc, 0, &one, &tm);
    if(ret != SUCCESS)
        return ret;

    for(curr_second = tm.second; curr_second == tm.second;)
        rtc_get_time(rtc, 0, &one, &tm);

    curr_second = tm.second;

    for(loops = 0; curr_second == tm.second; ++loops)
        rtc_get_time(rtc, 0, &one, &tm);

    *clk = 816 * loops;

    return SUCCESS;
}
