/*
    OS entry point

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, 2011-2017.


    TODO - BOOT PROCESS
    - look for additional mounts in /etc/mnttab (plenty to do to reach this point!)
*/

#include <kernel/housekeeper.h>
#include <kernel/include/boot.h>
#include <kernel/include/defs.h>
#include <kernel/include/device/block.h>
#include <kernel/include/device/memconsole.h>
#include <kernel/include/device/partition.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/platform.h>
#include <kernel/include/preempt.h>
#include <kernel/include/sched.h>
#include <kernel/include/tick.h>
#include <kernel/include/memory/extents.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/net/net.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>
#include <klibc/include/strings.h>
#include <monitor/include/monitor.h>


const char * const g_warmup_message = "\n  \\   ayumos"
                                      "\n  /\\  Stuart Wallace, 2011-2017.\n";


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
        boot_early_fail(BOOT_FAIL_PLATFORM_INIT);

    if(plat_mem_detect() != SUCCESS)    /* Detect installed RAM, initialise memory extents  */
        boot_early_fail(BOOT_FAIL_MEMORY_DETECT);

    /* Initialise kernel slabs */
    slab_init(ALIGN_NEXT(&_ebss, SLAB_SIZE_LOG2), SLAB_RESERVED_MEM);

    /* Initialise kernel heap */
    kmeminit((u8 *) ALIGN_NEXT(&_ebss, SLAB_SIZE_LOG2) + SLAB_RESERVED_MEM,
             mem_get_highest_addr(MEM_EXTENT_KERN | MEM_EXTENT_RAM) - KERNEL_STACK_LEN);

    /* Initialise user heap.  Place it in the largest user RAM extent. */
    ramext = mem_get_largest_extent(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    umeminit(ramext->base, ramext->base + ramext->len);

	/* By default, all exceptions cause a context-dump followed by a halt. */
	cpu_irq_init_table();

    /* Initialise device tree */
	if(dev_init() != SUCCESS)
        boot_early_fail(BOOT_FAIL_DEVICE_INIT);

	/*
        It's not yet possible to initialise the real (platform) console because devices haven't
        been enumerated and interrupts are disabled.  In the meantime, create a temporary in-memory
        kernel console device to capture output from the boot process.
    */

    if(early_boot_console_init() != SUCCESS)
        boot_early_fail(BOOT_FAIL_EARLY_CONSOLE_INIT);

    printf("%s\nplatform: %s\n", g_warmup_message, plat_get_name());

    printf("%uMB RAM detected\n", (mem_get_total_size(MEM_EXTENT_USER | MEM_EXTENT_RAM)
            + mem_get_total_size(MEM_EXTENT_KERN | MEM_EXTENT_RAM)) >> 20);

    /* === Initialise peripherals - phase 2 === */
    if(dev_enumerate() != SUCCESS)
        boot_early_fail(BOOT_FAIL_DEVICE_ENUMERATE);

    /* Initialise the (real) console */
    if(plat_console_init() != SUCCESS)
        boot_early_fail(BOOT_FAIL_CONSOLE_INIT);

    ret = sched_init("[sys]");      /* Init scheduler and create system process */

    /*
        Enable interrupts and continue booting
    */
    preempt_enable();

    /*
        Close the early-boot in-memory console: this has the effect of dumping its contents to the
        real console.
    */
    early_boot_console_close();

    /* Activate red LED while the boot process continues */
	plat_led_off(LED_ALL);
	plat_led_on(LED_RED);

    /*
        Device enumeration is done; interrupts are enabled, and the console should be functional.
        Booting continues...
    */

    /*
        Zero user RAM extents.
        Disabled for now because it slows the boot process and makes debugging harder.
    */
/*
    put("Clearing user RAM: ");
    mem_zero_extents(MEM_EXTENT_USER | MEM_EXTENT_RAM);
    puts("done");
*/

    /* Initialise the block cache, then scan mass-storage devices for partitions */
    block_cache_init(2039);

#ifdef WITH_DRV_MST_PARTITION
    partition_init();
#endif /* WITH_DRV_MST_PARTITION */

#ifdef WITH_MASS_STORAGE
    boot_list_mass_storage();
#endif /* WITH_MASS_STORAGE */

#ifdef WITH_DRV_MST_PARTITION
    boot_list_partitions();
#endif /* WITH_DRV_MST_PARTITION */

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

#ifdef WITH_RTC
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
#endif /* WITH_RTC */

    /* Create housekeeper process */
//    proc_create(0, 0, "[hk]", NULL, housekeeper, 0, 0, PROC_TYPE_KERNEL, NULL, NULL);

#ifdef WITH_NETWORKING
    /* Initialise networking system */
    ret = net_init();
    if(ret != SUCCESS)
        printf("net: init failed: %s\n", kstrerror(ret));
#endif /* WITH_NETWORKING */

    /* Startup complete - activate green LED */
	plat_led_off(LED_RED);
	plat_led_on(LED_GREEN);

	monitor();      /* start interactive "shell" thing */

	cpu_halt();		/* should never be reached */
}
