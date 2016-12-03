/*
    OS entry point

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, 2011-2015.


    TODO - BOOT PROCESS
    - look for additional mounts in /etc/mnttab (plenty to do to reach this point!)
*/

#include <kernel/boot.h>
#include <kernel/device/block.h>
#include <kernel/device/memconsole.h>
#include <kernel/device/partition.h>
#include <kernel/fs/vfs.h>
#include <kernel/housekeeper.h>
#include <kernel/include/defs.h>
#include <kernel/include/preempt.h>
#include <kernel/memory/extents.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/memory/slab.h>
#include <kernel/net/net.h>
#include <kernel/platform.h>
#include <kernel/sched.h>
#include <kernel/tick.h>
#include <kernel/util/kutil.h>
#include <monitor/monitor.h>
#include <stdio.h>
#include <strings.h>


const char * const g_warmup_message = "\n  \\   ayumos"
                                      "\n  /\\  Stuart Wallace, 2011-2016.\n";


void _main()
{
    mem_extent_t *ramext;
    u8 sn[6];
    u32 cpu_clk_hz = 0;
    rtc_time_t tm;
    s32 ret;

    /*
        This section runs with interrupts disabled.  The boot console is not available in this
        section.
    */
    preempt_disable();

    /* Copy kernel read/write data areas into kernel RAM */
    memcpy(&_sdata, &_etext, &_edata - &_sdata);        /* Copy .data section to kernel RAM */
    bzero(&_sbss, &_ebss - &_sbss);                     /* Initialise .bss section          */

    /* Begin platform initialisation */
    if(plat_init() != SUCCESS)
        boot_early_fail(1);

    if(plat_mem_detect() != SUCCESS)    /* Detect installed RAM, initialise memory extents  */
        boot_early_fail(2);

    /* Initialise kernel slabs */
    slab_init(&_ebss);                  /* Slabs sit after the .bss section */

    /* Initialise kernel heap */
    kmeminit(g_slab_end, mem_get_highest_addr(MEM_EXTENT_KERN | MEM_EXTENT_RAM) - KERNEL_STACK_LEN);

    /* Initialise user heap.  Place it in the largest user RAM extent. */
    ramext = mem_get_largest_extent(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    umeminit(ramext->base, ramext->base + ramext->len);

	/* By default, all exceptions cause a context-dump followed by a halt. */
	cpu_irq_init_table();

    /* Initialise device tree */
	if(dev_init() != SUCCESS)
        boot_early_fail(3);

	/*
        It's not yet possible to initialise the real (platform) console because devices haven't
        been enumerated and interrupts are disabled.  In the meantime, create a temporary in-memory
        kernel console device to capture output from the boot process.
    */

    if(early_boot_console_init() != SUCCESS)
        boot_early_fail(4);

    printf("%s\nplatform: %s\n", g_warmup_message, plat_get_name());

    printf("%uMB RAM detected\n", (mem_get_total_size(MEM_EXTENT_USER | MEM_EXTENT_RAM)
            + mem_get_total_size(MEM_EXTENT_KERN | MEM_EXTENT_RAM)) >> 20);

    /* === Initialise peripherals - phase 2 === */
    if(dev_enumerate() != SUCCESS)
        boot_early_fail(5);

    /* Initialise the console */
    if(plat_console_init() != SUCCESS)
        boot_early_fail(6);

    ret = sched_init("[sys]");      /* Init scheduler and create system process */

    /*
        Enable interrupts and continue booting
    */
    preempt_enable();

    /* Copy the contents of the temporary console to the real console; close the temp console. */
    early_boot_console_close();

    /* Activate red LED while the boot process continues */
	plat_led_off(LED_ALL);
	plat_led_on(LED_RED);

    /*
        Device enumeration is done; interrupts are enabled, and the console should be functional.
        Booting continues...
    */

    /* Zero any user RAM extents.  This happens after init'ing the DUART, because beeper. */
/*
    put("Clearing user RAM: ");
    mem_zero_extents(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    puts("done");
*/

    /* Initialise the block cache, then scan mass-storage devices for partitions */
    block_cache_init(2039);
    partition_init();

    boot_list_mass_storage();
    boot_list_partitions();

    /* ret is set by the call to sched_init(), above */
    if(ret != SUCCESS)
        printf("sched: init failed: %s\n", kstrerror(ret));

    ret = vfs_init();
	if(ret != SUCCESS)
		printf("vfs: init failed: %s\n", kstrerror(ret));

    /* Display approximate CPU clock speed */
    if(plat_get_cpu_clock(&cpu_clk_hz) == SUCCESS)
        printf("\nCPU fclk ~%2u.%uMHz\n", cpu_clk_hz / 1000000, (cpu_clk_hz % 1000000) / 100000);

    /* Initialise tick handler */
    tick_init();

    /* Display memory information */
	printf("%u bytes of kernel heap memory available\n"
           "%u bytes of user memory available\n", kfreemem(), ufreemem());

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

        if((time_iso8601(&tm, timebuf, sizeof(timebuf)) == SUCCESS) &&
            (date_long(&tm, datebuf, sizeof(datebuf)) == SUCCESS))
            printf("%s %s\n", timebuf, datebuf);
        else
            puts("Date/time invalid - please set clock");
    }

    /* Create housekeeper process */
//    proc_create(0, 0, "[hk]", NULL, housekeeper, 0, 0, PROC_TYPE_KERNEL, NULL, NULL);

    /* Initialise networking system */
    ret = net_init();
    if(ret != SUCCESS)
        printf("net: init failed: %s\n", kstrerror(ret));

    /* Startup complete - activate green LED */
	plat_led_off(LED_RED);
	plat_led_on(LED_GREEN);

	monitor();      /* start interactive "shell" thing */

	cpu_halt();		/* should never be reached */
}
