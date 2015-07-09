#include <stdio.h>
#include <strings.h>
#include "ds17485.h"
#include "duart.h"

#include "monitor/monitor.h"

#include "cpu/utilities.h"
#include "device/device.h"
#include "kutil/kutil.h"
#include "memory/kmalloc.h"
#include "fs/vfs.h"

const char * const g_warmup_message = "\n\n68010 computer system\n"
									  "(c) Stuart Wallace, 2011-2015\n";

u32 test = 0x12345678;
u32 zero = 0;

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

extern u8 _sdata, _edata, _sbss, _ebss, _stext, _etext;

void _main()
{
	/* At boot, interrupts are disabled */

    struct rtc_time tm;
    char sn[6], timebuf[12], datebuf[32];

	kmeminit();

	/* Install default handlers for all CPU exceptions */
	__cpu_exc_install_default_handlers();

	duart_init();

	led_off(LED_RED | LED_GREEN);
	led_on(LED_RED);

	ds17485_init();

    puts(g_warmup_message);

    printf("------- Memory map -------\n"
           ".data: %08x - %08x\n"
           ".bss : %08x - %08x\n"
           ".text: %08x - %08x\n"
           "--------------------------\n\n",
           &_sdata, &_edata, &_sbss, &_ebss, &_stext, &_etext);

    ds17485_get_serial_number(sn);

    /* Copy .data section to RAM */
    memcpy(&_sdata, &_etext, &_edata - &_sdata);

    /* Initialise .bss section */
    bzero(&_sbss, &_ebss - &_sbss);

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

	led_off(LED_RED);
	led_on(LED_GREEN);

    sched_init();
    cpu_enable_interrupts();
	monitor();

	__cpu_halt();		/* should never be reached */
}

