#include <stdio.h>
#include <strings.h>

#include <cpu/utilities.h>
#include <device/device.h>
#include <ds17485.h>
#include <duart.h>
#include <fs/vfs.h>
#include <include/defs.h>
#include <kutil/kutil.h>
#include <memory/kmalloc.h>
#include <memory/ramdetect.h>
#include <memory/slab.h>
#include <monitor/monitor.h>


const char * const g_warmup_message = "\n\n68010 computer system\n"
									  "(c) Stuart Wallace, 2011-2015\n";


void detect_clock_freq()
{
    /* Ensure that interrupts are disabled before entering this section */
    struct rtc_time tm;
    u32 loops;
    u8 curr_second;

    /* Wait for the next second to start */
    ds17485_get_time(&tm);
    for(curr_second = tm.second; curr_second == tm.second;)
        ds17485_get_time(&tm);
    curr_second = tm.second;

    for(loops = 0; curr_second == tm.second; ++loops)
        ds17485_get_time(&tm);

    printf("Loop counter final value is %u\n", loops);

    // CPU fclk/MHz ~= loops/1420
    // CPU fclk/Hz ~= 700 * loops
}


void _main()
{
    struct rtc_time tm;
    char sn[6], timebuf[12], datebuf[32];

	/* === Initialise CPU === */

    cpu_disable_interrupts();   /* Just in case we were called manually */

	/* By default, all exceptions cause a context-dump followed by a halt. */
	cpu_exc_install_default_handlers();

    /* === Initialise memory === */

    memcpy(&_sdata, &_etext, &_edata - &_sdata);    /* Copy .data section to kernel RAM */
    bzero(&_sbss, &_ebss - &_sbss);                 /* Initialise .bss section          */
    ram_detect();                                   /* Find out how much RAM we have    */
    slab_init(&_ebss);                              /* Slabs sit after the .bss section */
	kmeminit(g_slab_end, (void *) OS_STACK_BOTTOM); /* Initialise kernel heap           */

    /* === Initialise peripherals === */

	duart_init();

    /* Activate red LED while the boot process continues */
	led_off(LED_RED | LED_GREEN);
	led_on(LED_RED);

    puts(g_warmup_message);

    printf("%uMB RAM detected\n", g_ram_top >> 20);

    /* Initialise RTC */
	ds17485_init();
    ds17485_get_serial_number(sn);

    printf("RTC model %02X - serial number %02X%02X%02X%02X%02X%02X\n",
           ds17485_get_model_number(), sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);

    ds17485_get_time(&tm);
    time_iso8601(&tm, timebuf, sizeof(timebuf));
    date_long(&tm, datebuf, sizeof(datebuf));
    printf("%s %s\n", timebuf, datebuf);

    detect_clock_freq();

	/* Register drivers and initialise devices */
	puts("Initialising devices and drivers");
	driver_init();

	printf("%u bytes of heap memory available\n", kfreemem());

	if(vfs_init())
		puts("VFS failed to initialise");

    sched_init();
    cpu_enable_interrupts();

    /* Startup complete - activate green LED */
	led_off(LED_RED);
	led_on(LED_GREEN);

	monitor();      /* start interactive "shell" thing */

	cpu_halt();		/* should never be reached */
}
