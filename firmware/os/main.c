/*
    TODO - BOOT PROCESS
    - look for additional mounts in /etc/mnttab (plenty to do to reach this point!)

    FIXME - OS_STACK_BOTTOM is platform-specific and shouldn't be in here
*/

#include <platform/platform.h>
#include <stdio.h>
#include <strings.h>

#include <device/devctl.h>
#include <device/device.h>
#include <fs/vfs.h>
#include <include/defs.h>
#include <kutil/kutil.h>
#include <memory/extents.h>
#include <memory/kmalloc.h>
#include <memory/slab.h>
#include <monitor/monitor.h>
#include <kernel/sched.h>

const char * const g_warmup_message = "\n  \\   { ayumos }"
                                      "\n  /\\  Stuart Wallace, 2011-2015.\n";



/*
    early_boot_fail() - report an "early failure" by flashing the motherboard LEDs.

    This code assumes that interrupts have not yet been enabled.  This function does not return.

    TODO: move this somewhere else.
*/
void early_boot_fail(ku32 code)
{
    /* TODO - remove hardwired loops */
    while(1)
    {
        u32 i, j;

        for(j = 250000; j; j--)
            plat_led_off(LED_ALL);

        for(i = 0; i < code; ++i)
        {
            u32 j;

            plat_led_on(LED_RED);
            for(j = 100000; j; j--)
                ;

            plat_led_off(LED_ALL);
            for(j = 100000; j; j--)
                ;
        }
    }
}


void _main()
{
    mem_extent_t *ramext;
    u8 sn[6];
    u32 cpu_clk_hz = 0;
    rtc_time_t tm;

    cpu_disable_interrupts();   /* Just in case we were called manually */

    /* === Initialise memory === */
    memcpy(&_sdata, &_etext, &_edata - &_sdata);        /* Copy .data section to kernel RAM */
    bzero(&_sbss, &_ebss - &_sbss);                     /* Initialise .bss section          */
    slab_init(&_ebss);                                  /* Slabs sit after the .bss section */
	kmeminit(g_slab_end, (void *) OS_STACK_BOTTOM);     /* Initialise kernel heap           */

	/* By default, all exceptions cause a context-dump followed by a halt. */
	cpu_init_interrupt_handlers();

    /* === Initialise peripherals - phase 1 === */
    if(plat_init() != SUCCESS)
        early_boot_fail(1);

    if(plat_mem_detect() != SUCCESS)        /* Detect installed RAM */
        early_boot_fail(2);

    /* Initialise DUART.  This has the side-effect of shutting the goddamned beeper up. */
    if(plat_console_init() != SUCCESS)
        early_boot_fail(3);

    /*
        At this point, minimal configuration has been done.  The scheduler is not yet running,
        but we can now initialise non-critical peripherals and start greeting the user.
    */
    puts(g_warmup_message);

    printf("%uMB RAM detected\n", (mem_get_total_size(MEM_EXTENT_USER | MEM_EXTENT_RAM)
            + mem_get_total_size(MEM_EXTENT_KERN | MEM_EXTENT_RAM)) >> 20);

    /* Activate red LED while the boot process continues */
	plat_led_off(LED_ALL);
	plat_led_on(LED_RED);

    /* Zero any user RAM extents.  This happens after init'ing the DUART, because beeper. */
    put("Zeroing user RAM: ");
    mem_zero_extents(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    puts("done");

    /* Initialise user heap.  Place it in the largest user RAM extent. */
    ramext = mem_get_largest_extent(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    umeminit(ramext->base, ramext->base + ramext->len);

    /* === Initialise peripherals - phase 2 === */
    if(dev_enumerate() != SUCCESS)
        early_boot_fail(4);

	/* Register drivers and initialise devices */
	puts("Initialising devices and drivers");
	driver_init();

	if(vfs_init() != SUCCESS)
		puts("VFS failed to initialise");

    /* Display approximate CPU clock speed */
    if(plat_get_cpu_clock(&cpu_clk_hz) == SUCCESS)
        printf("\nCPU fclk ~%2u.%uMHz\n", cpu_clk_hz / 1000000, (cpu_clk_hz % 1000000) / 100000);

    /* Display memory information */
	printf("%u bytes of kernel heap memory available\n"
           "%u bytes of user memory available\n", kfreemem(),
                mem_get_total_size(MEM_EXTENT_USER | MEM_EXTENT_RAM));

    /* Display platform serial number */
    if(plat_get_serial_number(sn) == SUCCESS)
    {
        printf("Hardware serial number %02X%02X%02X%02X%02X%02X\n",
                sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);
    }

    /* Display the current date and time */
    if(get_time(&tm) == SUCCESS)
    {
        char timebuf[12], datebuf[32];

        time_iso8601(&tm, timebuf, sizeof(timebuf));
        date_long(&tm, datebuf, sizeof(datebuf));
        printf("%s %s\n", timebuf, datebuf);
    }

    sched_init();
    cpu_enable_interrupts();

    /* Startup complete - activate green LED */
	plat_led_off(LED_RED);
	plat_led_on(LED_GREEN);

	monitor();      /* start interactive "shell" thing */

	cpu_halt();		/* should never be reached */
}
