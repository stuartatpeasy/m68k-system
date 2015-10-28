/*
	Monitor application command handlers

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "monitor.h"


/*
    date

    Get/set the date
*/
MONITOR_CMD_HANDLER(date)
{
    rtc_time_t tm;
    dev_t *dev;

    if(num_args > 1)
        return EINVAL;

    /* Find an RTC */
    dev = dev_find("rtc0");
    if(dev == NULL)
        return ENODEV;

    if(num_args  == 0)
    {
        char timebuf[12], datebuf[32];

        dev->read(dev, 0, 1, &tm);

        time_iso8601(&tm, timebuf, sizeof(timebuf));
        date_long(&tm, datebuf, sizeof(datebuf));
        printf("%s %s\n", timebuf, datebuf);
    }
    else if(num_args == 1)
    {
        /*
            Set the date and time.  Acceptable format:
                date YYYYMMDDHHMMSS
        */
        s32 ret = rtc_time_from_str(args[0], &tm);
        if(ret != SUCCESS)
            return ret;

        return dev->write(dev, 0, 1, &tm);
    }
    else
        return EINVAL;

    return SUCCESS;
}


/*
    dfu <len> <checksum>

    Re-flash boot ROM
*/
MONITOR_CMD_HANDLER(dfu)
{
	u32 len, buffer_len, cksum_sent, cksum_calculated, i, ret;
	s8 *data, *endptr;

	if(num_args != 2)
		return EINVAL;

	len = strtoul(args[0], &endptr, 0);
	if(*endptr || !len)
		return EINVAL;

    cksum_sent = strtoul(args[1], &endptr, 0);
    if(*endptr || !len)
        return EINVAL;

    /*
        The dfu() function requires an even number of bytes, so - if the new firmware image is an
        odd number of bytes in length, we allocate an extra byte to the data buffer and add a
        padding byte (0x00).
    */
    buffer_len = (len & 1) ? len + 1 : len;

	if((data = umalloc(buffer_len)) == NULL)
		return ENOMEM;

    printf("Send %u bytes\n", len);
	for(i = 0; i < len; i++)
		data[i] = plat_console_getc();

    if(len & 1)
        data[i] = 0x00;     /* Add padding byte - see above */

    cksum_calculated = fletcher16(data, len);
    if(cksum_calculated != cksum_sent)
        return ECKSUM;

	if((ret = dfu((ku16 *) data, buffer_len)))
		printf("Firmware update failed: %s\n", kstrerror(ret));

    ufree(data);

	return SUCCESS;
}


/*
    disassemble <start> [<count>]

    Disassemble <count> (default 256) bytes of code starting at <start>.
*/
MONITOR_CMD_HANDLER(disassemble)
{
	u32 num_bytes, start;
	s8 *endptr;
	u16 *addr;
	s8 line[80], instr_printed;

	if(!num_args || (num_args > 2))
		return EINVAL;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr || (start & 1))
		return EINVAL;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || (num_bytes < 2) || (num_bytes & 1))
			return EINVAL;
	}
	else num_bytes = 256;

	for(addr = (u16 *) start; addr < (u16 *) (start + num_bytes);)
	{
		u16 *addr_ = addr;

		disassemble(&addr_, line);
		instr_printed = 0;
		while(addr < addr_)
		{
			s8 i;

			if(!instr_printed)
				printf("%06x: ", (u32) addr);
			else
				printf("        ");

			for(i = 0; i < 3; ++i)
				if(addr < addr_)
					printf("%04x ", *addr++);
				else
					printf("     ");

			if(!instr_printed++)
				printf(" %s\n", line);
			else
				puts("");
		}
	}

	return SUCCESS;
}


/*
    dump_mem() - helper for dump[h|w] commands
*/
s32 dump_mem(ks32 start, ks32 num_bytes, ks8 word_size)
{
    return dump_hex((void *) start, word_size, 0, num_bytes);
}


/*
	dump <start> [<offset>]

	Dump bytes.  Offset (if supplied) must be greater than zero.
*/
MONITOR_CMD_HANDLER(dump)
{
	u32 start, num_bytes;
	s8 *endptr;

	if(!num_args || (num_args > 2))
		return EINVAL;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return EINVAL;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || (num_bytes < 1))
			return EINVAL;
	}
	else num_bytes = CMD_DUMP_DEFAULT_NUM_BYTES;

	return dump_mem(start, num_bytes, 1);
}


/*
	dumph <start> [<offset>]

	Dump half-words.  Start must be even; offset (if supplied) must be even and greater than zero.
*/
MONITOR_CMD_HANDLER(dumph)
{
	u32 start, num_bytes;
	s8 *endptr;

	if(!num_args || (num_args > 2))
		return EINVAL;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return EINVAL;

	if(start & 1)
		return EINVAL;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || !num_bytes || (num_bytes & 1))
			return EINVAL;
	}
	else num_bytes = CMD_DUMP_DEFAULT_NUM_BYTES;

	return dump_mem(start, num_bytes, 2);
}


/*
	dumpw <start> [<offset>]

	Dump words.  Start must be divisible by 4; offset (if supplied) must divisible by four and
	greater than zero.
*/
MONITOR_CMD_HANDLER(dumpw)
{
	u32 start, num_bytes;
	s8 *endptr;

	if(!num_args || (num_args > 2))
		return EINVAL;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return EINVAL;

	if(start & 3)
		return EINVAL;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || !num_bytes || (num_bytes & 3))
			return EINVAL;
	}
	else num_bytes = CMD_DUMP_DEFAULT_NUM_BYTES;

	return dump_mem(start, num_bytes, 4);
}


/*
    echo

    Echo all args back to the client.
*/
MONITOR_CMD_HANDLER(echo)
{
    s32 i;

    for(i = 0; i < num_args; ++i)
    {
        if(i)
            putchar(' ');
        put(args[i]);
    }

    if(num_args)
        puts("");

    return SUCCESS;
}


MONITOR_CMD_HANDLER(fill)
{
	u32 count, data;
	u8 *start;
	s8 *endptr;

	if(num_args != 3)
		return EINVAL;

	start = (u8 *) strtoul(args[0], &endptr, 0);
	if(*endptr)
		return EINVAL;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr | !count)
		return EINVAL;

	data = strtoul(args[2], &endptr, 0);
	if((*endptr) || (data > 0xff))
		return EINVAL;

	while(count--)
		*((u8 *) start++) = (u8) data;

	return SUCCESS;
}


MONITOR_CMD_HANDLER(fillh)
{
	u32 count, data;
	u16 *start;
	s8 *endptr;

	if(num_args != 3)
		return EINVAL;

	start = (u16 *) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) start & 0x1))
		return EINVAL;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr || !count)
		return EINVAL;

	data = strtoul(args[2], &endptr, 0);
	if((*endptr) || (data > 0xffff))
		return EINVAL;

	while(count--)
		*start++ = (u16) data;

	return SUCCESS;
}


MONITOR_CMD_HANDLER(fillw)
{
	u32 *start, count, data;
	s8 *endptr;

	if(num_args != 3)
		return EINVAL;

	start = (u32 *) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) start & 0x3))
		return EINVAL;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr | !count)
		return EINVAL;

	data = strtoul(args[2], &endptr, 0);
	if(*endptr)
		return EINVAL;

	while(count--)
		*start++ = data;

	return SUCCESS;
}


MONITOR_CMD_HANDLER(free)
{
    UNUSED(num_args);
    UNUSED(args);

	printf("kernel: %9d bytes free\n"
           "  user: %9d bytes free\n", kfreemem(), ufreemem());
	return SUCCESS;
}


/*
	go <addr>

	Jump to the specified address.
*/
MONITOR_CMD_HANDLER(go)
{
	s8 *endptr;
	void (*addr)();

	if(num_args != 1)
		return EINVAL;

	addr = (void (*)()) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) addr & 1))
		return EINVAL;

	addr();

	return SUCCESS;
}


/*
	help

	Display helpful text.
*/
MONITOR_CMD_HANDLER(help)
{
    UNUSED(num_args);
    UNUSED(args);

	puts("Available commands (all can be abbreviated):\n\n"
          "date [<newdate>]\n"
          "    If no argument is supplied, print the current date and time.  If date is specified\n"
          "    in YYYYMMDDHHMMSS format, set the RTC date and time accordingly.\n\n"
		  "dfu <size> <checksum>\n"
		  "    Receive <size> bytes and re-flash the firmware ROMs with this data.  <size> must\n"
		  "    be an even number.  <checksum> is the Fletcher16 checksum of the data.\n\n"
		  "disassemble <start> [<count>]\n"
		  "    Disassemble <count> bytes starting at offset <start>.  <count> must be an\n"
		  "    even number greater than or equal to two.  <start> must be an even number.\n\n"
		  "dump[h|w] <address> [<count>]\n"
		  "    Dump <count> bytes (dump), half-words (dumph) or words (dumpw), starting at <address>\n\n"
		  "echo [<arg> ...]\n"
		  "    Echo all arguments, each separated by a single space character, to the console\n\n"
		  "fill[h|w] <start> <count> <value>\n"
		  "    Fill <count> bytes (fill), half-words (fillh) or words (fillw), starting at <address>\n"
		  "    with <value>\n\n"
		  "free\n"
		  "    Show the number of bytes of available heap memory\n\n"
		  "go <address>\n"
		  "    Begin executing code at <address>, which must be an even number\n\n"
		  "help\n"
		  "    Display this text\n\n"
		  "history\n"
		  "    Display command history\n\n"
		  "id\n"
		  "    Display device identity\n\n"
		  "ls [<path>]\n"
		  "    List directory contents\n\n"
		  "map\n"
		  "    Display memory map\n\n"
		  "mount [<dev> <fstype> <mountpoint>]\n"
		  "    With no arguments, list current mounts.  With arguments, mount block device <dev>\n"
		  "    containing a file system of type <fstype> at <mountpoint>\n\n"
		  "raw\n"
		  "    Dump raw characters in hex format.  Ctrl-A stops.\n\n"
		  "rootfs [<partition> <type>]\n"
		  "    Set/read root partition in BIOS data area.\n\n"
		  "schedule\n"
		  "    Start task scheduler\n\n"
		  "serial\n"
		  "    Configure serial line discipline:\n"
		  "        serial echo off - disable character echo\n"
		  "        serial echo on  - enable character echo\n\n"
		  "slabs\n"
		  "    Display slab allocation status\n\n"
		  "srec\n"
		  "    Start the upload of an S-record file\n\n"
		  "upload <count>\n"
		  "    Receive <count> bytes and place them in memory\n\n"
		  "write[h|w] <address> <data>\n"
		  "    Write <data> to to memory at <address>\n\n"
		);
	return SUCCESS;
}


/*
    history

    Display command history
*/
MONITOR_CMD_HANDLER(history)
{
    u32 i;
    UNUSED(num_args);
    UNUSED(args);

    printf("next = %u\n", g_hist->next);
    for(i = 0; i < history_get_len(g_hist); ++i)
        printf("item[%u] = %s\n", i, g_hist->item[i] ? g_hist->item[i] : "");

    for(i = 0; i < history_get_len(g_hist); ++i)
    {
        const char *cmd = history_get_at(g_hist, i);
        printf("%3i: %s\n", i, cmd == NULL ? "" : cmd);
    }

    return SUCCESS;
}


/*
    id

    Display device identity
*/
MONITOR_CMD_HANDLER(id)
{
    UNUSED(num_args);
    UNUSED(args);

    puts(OS_NAME " v" OS_VERSION_STR " on " CPU_NAME ", build date " __DATE__ " " __TIME__);
    return SUCCESS;
}


/*
    ls

    List directory contents
*/
MONITOR_CMD_HANDLER(ls)
{
    vfs_dirent_t dirent;
    char perms[11];

    perms[10] = '\0';

    /* For now we require a path arg */
    if(num_args == 1)
    {
        s32 ret;

        ret = vfs_lookup(args[0], &dirent);
        if(ret != SUCCESS)
        {
            puts(kstrerror(ret));
            return SUCCESS;
        }

        if(dirent.type == FSNODE_TYPE_DIR)
        {
            /* Iterate directory */
            vfs_dir_ctx_t ctx;

            ret = vfs_open_dir(args[0], &ctx);
            if(ret != SUCCESS)
            {
                puts(kstrerror(ret));
                return SUCCESS;
            }

            while((ret = vfs_read_dir(&ctx, &dirent, NULL)) != ENOENT)
            {
                printf("%9d %s %9d %s\n", dirent.first_node, vfs_dirent_perm_str(&dirent, perms),
                       dirent.size, dirent.name);
            }

            if(ret != ENOENT)
                puts(kstrerror(ret));

            vfs_close_dir(&ctx);
        }
        else
        {
            /* Print single file */
            printf("%9d %s %9d %s\n", dirent.first_node, vfs_dirent_perm_str(&dirent, perms),
                   dirent.size, dirent.name);
        }
    }
    else
        return EINVAL;

    return SUCCESS;
}


/*
    map

    Display memory map
*/
MONITOR_CMD_HANDLER(map)
{
    UNUSED(num_args);
    UNUSED(args);

    /* FIXME */
    puts("This is broken at the moment.  Please check back later.");
/*
    printf("--------------- Memory map ---------------\n"
           ".data : %08x - %08x (%u bytes)\n"
           ".bss  : %08x - %08x (%u bytes)\n"
           "slabs : %08x - %08x (%u bytes)\n"
           "kheap : %08x - %08x (%u bytes)\n"
           "kstack: %08x - %08x (%u bytes)\n"
           "uram  : %08x - %08x (%u bytes)\n"
           ".text : %08x - %08x (%u bytes)\n"
           "------------------------------------------\n\n",
           &_sdata, &_edata, &_edata - &_sdata,
           &_sbss, &_ebss, &_ebss - &_sbss,
           g_slab_base, g_slab_end, g_slab_end - g_slab_base,
           g_slab_end, OS_STACK_BOTTOM, (void *) OS_STACK_BOTTOM - g_slab_end,
           OS_STACK_BOTTOM, OS_STACK_TOP, OS_STACK_TOP - OS_STACK_BOTTOM,
           USER_RAM_START, g_ram_top, g_ram_top - USER_RAM_START,
           &_stext, &_etext, &_etext - &_stext);
*/
    return SUCCESS;
}


/*
    mount

    Display mount table, or mount a filesystem at the specified location.
    Syntax: mount [<dev> <fstype> <mountpoint>]
*/
MONITOR_CMD_HANDLER(mount)
{
    UNUSED(num_args);
    UNUSED(args);

    if(num_args == 0)
    {
        /* Display mount table */
        u32 u;
        for(u = 0; u < g_max_mounts; ++u)
        {
            if(g_mount_table[u].vfs)
                printf("%-10s %-10s %s\n", g_mount_table[u].vfs->dev->name,
                       g_mount_table[u].vfs->driver->name, g_mount_table[u].mount_point);
        }

    }
    else if(num_args == 3)
    {
        /* Mount filesystem */
        return ENOSYS;
    }
    else
        return EINVAL;

    return SUCCESS;
}


/*
	raw

	Hex-dump received characters until ctrl-A (0x01) is received.
*/
MONITOR_CMD_HANDLER(raw)
{
    UNUSED(num_args);
    UNUSED(args);

	puts("Dumping raw output.  Use ctrl-A to stop.\n");
	for(;;)
	{
		char c = plat_console_getc();
		printf("0x%02x ", c);

		if(c == 0x01 /* ctrl-A */)
			break;
	}

	puts("\n");
	return SUCCESS;
}


/*
    rootfs

    Set/read root partition name in BIOS data area
*/
MONITOR_CMD_HANDLER(rootfs)
{
    nvram_bpb_t bpb;
    s32 ret;

    if((num_args != 0) && (num_args != 2))
        return EINVAL;

    if(num_args == 0)
    {
        /* Read and display current root filesystem setting from BPB */
        ret = nvram_bpb_read(&bpb);
        if(ret != SUCCESS)
            return ret;

        u32 u;
        for(u = 0; bpb.rootfs[u] && (u < sizeof(bpb.rootfs)); ++u)
            putchar(bpb.rootfs[u]);

        putchar(' ');

        for(u = 0; bpb.fstype[u] && (u < sizeof(bpb.fstype)); ++u)
            putchar(bpb.fstype[u]);

        putchar('\n');
    }
    else
    {
        /* Set root filesystem in BPB */
        if((strlen(args[0]) > sizeof(bpb.rootfs)) || (strlen(args[1]) > sizeof(bpb.fstype))
           || !vfs_get_driver_by_name(args[1]))
            return EINVAL;

        /*
            Optimistically ignore errors here.  If there's a checksum error,
            bbram_param_block_write() will fix it for us.  This doesn't mean there won't be
            corruption elsewhere in the BPB, though.
        */
        nvram_bpb_read(&bpb);

        strncpy(bpb.rootfs, args[0], sizeof(bpb.rootfs));
        strncpy(bpb.fstype, args[1], sizeof(bpb.fstype));

        return nvram_bpb_write(&bpb);
    }

    return SUCCESS;
}


/*
    sched

    Start scheduler
*/
MONITOR_CMD_HANDLER(schedule)
{
    UNUSED(num_args);
    UNUSED(args);

    plat_start_quantum();
    return SUCCESS;
}


/*
    serial

    Configure serial line discipline
*/
MONITOR_CMD_HANDLER(serial)
{
    if(num_args == 2)
    {
        if(!strcmp(args[0], "echo"))
        {
            /* Enable disable echo */
            if(!strcmp(args[1], "on"))
            {
                g_echo = 1;
                return SUCCESS;
            }
            else if(!strcmp(args[1], "off"))
            {
                g_echo = 0;
                return SUCCESS;
            }
        }
    }

    return EINVAL;
}


/*
    slabs

    Display slab allocation status
*/
MONITOR_CMD_HANDLER(slabs)
{
    u16 u;
    UNUSED(num_args);
    UNUSED(args);

    puts("------------- Slab dump -------------");
    for(u = 0; u < NSLABS; ++u)
    {
        ku8 au = g_slabs[u].alloc_unit;
        const void * const p = g_slabs[u].p;

        printf("%02u: %08x - %08x au=%u (%ux%ub)\n",
                u, p, p + SLAB_SIZE - 1, au, SLAB_SIZE >> au, 1 << au);
    }
    puts("-------------------------------------");

    return SUCCESS;
}


/*
	srec

	Receive data in S-record format and put it in memory somewhere
*/
MONITOR_CMD_HANDLER(srec)
{
    UNUSED(args);

	if(num_args)
		return EINVAL;

	struct srec_data s;

	if(srec(&s))
	{
		printf("S-record error at line %u: %s\n", s.line, srec_strerror(s.error));
		return SUCCESS;
	}

	printf("Uploaded %u bytes at address %p\nStart address %x\n",
				s.data_len, s.data, s.start_address);

	return SUCCESS;
}


/*
    test

    Used to trigger a test of some sort
*/
#include "include/list.h"
struct mydata
{
    int meh;
    int foo;
    int bar;
    list_t ll;
};


#include <kernel/process.h>
#include <include/elf.h>
MONITOR_CMD_HANDLER(test)
{
    if(num_args < 1)
        return EINVAL;

    ku32 testnum = strtoul(args[0], NULL, 0);

    if(testnum == 1)
    {
#if 0
        struct mydata
        x = {
            .meh = 1,
            .foo = 2,
            .bar = 3,
            .ll = LIST_INIT(x.ll)
        },
        y = {
            .meh = 10,
            .foo = 20,
            .bar = 30,
            .ll = LIST_INIT(y.ll)
        },
        z = {
            .meh = 100,
            .foo = 200,
            .bar = 300,
            .ll = LIST_INIT(z.ll)
        },
        *p;

        LINKED_LIST(mylist);

        list_append(&x.ll, &mylist);
        list_append(&y.ll, &mylist);
        list_append(&z.ll, &mylist);

        list_for_each_entry(p, &mylist, ll)
        {
            printf("foo is %d\n", p->foo);
        }
#endif
    }
    else if(testnum == 2)
    {
        const unsigned char testapp[] = {
          0x7f, 0x45, 0x4c, 0x46, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x34,
          0x00, 0x00, 0x01, 0x5c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x02, 0x00, 0x28,
          0x00, 0x09, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
          0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea, 0x00, 0x00, 0x00, 0xea, 0x00, 0x00, 0x00, 0x05,
          0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xea, 0x80, 0x00, 0x20, 0xea,
          0x80, 0x00, 0x20, 0xea, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x06,
          0x00, 0x00, 0x20, 0x00, 0x4e, 0x56, 0x00, 0x00, 0x48, 0xe7, 0x20, 0x34, 0x4b, 0xfa, 0x20, 0x72,
          0x70, 0x48, 0x24, 0x6d, 0x00, 0x14, 0x26, 0x6d, 0x00, 0x0c, 0x52, 0x8a, 0x48, 0x80, 0x30, 0x40,
          0x2f, 0x08, 0x48, 0x78, 0x00, 0x01, 0x4e, 0x93, 0x10, 0x12, 0x50, 0x8f, 0x66, 0xec, 0x24, 0x6d,
          0x00, 0x10, 0x74, 0x64, 0x48, 0x78, 0x00, 0x04, 0x4e, 0x92, 0x53, 0x82, 0x58, 0x8f, 0x66, 0xf4,
          0x48, 0x78, 0x00, 0x2a, 0x48, 0x78, 0x00, 0x01, 0x4e, 0x93, 0x50, 0x8f, 0x74, 0x64, 0x60, 0xe4,
          0x61, 0x00, 0xff, 0xb2, 0x2f, 0x00, 0x48, 0x78, 0x00, 0x00, 0x4e, 0x40, 0x4e, 0x40, 0x4e, 0x75,
          0x4e, 0x40, 0x4e, 0x75, 0x4e, 0x40, 0x4e, 0x75, 0x4e, 0x40, 0x4e, 0x75, 0x48, 0x65, 0x6c, 0x6c,
          0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00, 0x80, 0x00, 0x00, 0xdc, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xd0,
          0x80, 0x00, 0x00, 0xcc, 0x80, 0x00, 0x00, 0xdc, 0x12, 0x34, 0x56, 0x78, 0x47, 0x43, 0x43, 0x3a,
          0x20, 0x28, 0x47, 0x4e, 0x55, 0x29, 0x20, 0x35, 0x2e, 0x32, 0x2e, 0x30, 0x00, 0x00, 0x2e, 0x73,
          0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x2e, 0x72,
          0x6f, 0x64, 0x61, 0x74, 0x61, 0x00, 0x2e, 0x64, 0x61, 0x74, 0x61, 0x2e, 0x72, 0x65, 0x6c, 0x2e,
          0x72, 0x6f, 0x00, 0x2e, 0x67, 0x6f, 0x74, 0x00, 0x2e, 0x64, 0x61, 0x74, 0x61, 0x00, 0x2e, 0x62,
          0x73, 0x73, 0x00, 0x2e, 0x63, 0x6f, 0x6d, 0x6d, 0x65, 0x6e, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06,
          0x80, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x80, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, 0xdc,
          0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03,
          0x80, 0x00, 0x20, 0xea, 0x00, 0x00, 0x00, 0xea, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x20, 0xf0, 0x00, 0x00, 0x00, 0xf0,
          0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
          0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03,
          0x80, 0x00, 0x21, 0x08, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31,
          0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x21, 0x0c, 0x00, 0x00, 0x01, 0x0c,
          0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x30,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
          0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1d,
          0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
          0x00, 0x00, 0x00, 0x00
        };

        void * const addr = (void *) 0x100000;
        exe_img_t img;
        pid_t pid = 0;
        s32 ret;

        printf("copying %d bytes to %p\n", sizeof(testapp), addr);
        memcpy(addr, testapp, sizeof(testapp));
        ret = elf_load_exe(addr, sizeof(testapp), &img);
        if(ret != SUCCESS)
            printf("elf_load_exe() returned %u: %s\n", ret, kstrerror(ret));

        /* FIXME - there's currently no way to free an exe_img_t cleanly */

        ret = proc_create(0, 0, "testapp", img.entry_point, NULL, 1024, 0, &pid);
        printf("proc_create returned %d (%s); pid=%d\n", ret, kstrerror(ret), pid);

/*
        pid_t pid = 0;

        s32 ret = proc_create(0, 0, "testapp", (proc_main_t) 0x100000, NULL, 1024, 0, &pid);
        printf("proc_create() pid is %u; retval %u (%s)\n", pid, ret, kstrerror(ret));

        ret = proc_create(0, 0, "test2", (proc_main_t) 0x100100, NULL, 1024, 0, &pid);
        printf("proc_create() pid is %u; retval %u (%s)\n", pid, ret, kstrerror(ret));
*/
    }

    return SUCCESS;
}


/*
    upload

    Allocate memory, receive bytes, store them in the allocated region
*/
MONITOR_CMD_HANDLER(upload)
{
	u32 len;
	s8 *data, *data_, *endptr;

	if(num_args != 1)
		return EINVAL;

	len = strtoul(args[0], &endptr, 0);
	if(*endptr || !len)
		return EINVAL;

	if((data = kmalloc(len)) == NULL)
		return ENOMEM;

	for(data_ = data; len--;)
		*data_++ = plat_console_getc();

	printf("Uploaded %li bytes at %p\n", data_ - data, data);

	return SUCCESS;
}


/*
    write

    Write a byte to the specified address
*/
MONITOR_CMD_HANDLER(write)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return EINVAL;

	addr = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return EINVAL;

	data = strtoul(args[1], &endptr, 0);
	if((*endptr) || (data > 0xff))
		return EINVAL;

	*((u8 *) addr) = (u8) data;

	return SUCCESS;
}


/*
    writeh

    Write a half-word (16 bits) to the specified address
*/
MONITOR_CMD_HANDLER(writeh)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return EINVAL;

	addr = strtoul(args[0], &endptr, 0);
	if((*endptr) || (addr & 0x1))
		return EINVAL;

	data = strtoul(args[1], &endptr, 0);
	if((*endptr) || (data > 0xffff))
		return EINVAL;

	*((u16 *) addr) = (u16) data;

	return SUCCESS;
}


/*
    writew

    Write a word (32 bits) to the specified address
*/
MONITOR_CMD_HANDLER(writew)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return EINVAL;

	addr = strtoul(args[0], &endptr, 0);
	if((*endptr) || (addr & 0x3))
		return EINVAL;

	data = strtoul(args[1], &endptr, 0);
	if(*endptr)
		return EINVAL;

	*((u32 *) addr) = data;

	return SUCCESS;
}
