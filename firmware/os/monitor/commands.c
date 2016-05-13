/*
	Monitor application command handlers

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include <monitor/monitor.h>


/*
    arp

    Work with the ARP cache
*/
MONITOR_CMD_HANDLER(arp)
{
    if(num_args >= 1)
    {
        if(!strcmp(args[0], "list"))
        {
            arp_cache_item_t *item;
            extern time_t g_current_timestamp;
            char hw_addrbuf[24], proto_addrbuf[24];
            u32 n;
            u8 header;

            for(header = 0, n = 0; (item = arp_cache_get_item(n)) != NULL; ++n)
            {
                if(item->iface && (item->etime > g_current_timestamp))
                {
                    if(!header++)
                        puts("Iface  Hardware address   Protocol address   TTL");

                    net_print_addr(&item->hw_addr, hw_addrbuf, sizeof(hw_addrbuf));
                    net_print_addr(&item->proto_addr, proto_addrbuf, sizeof(proto_addrbuf));

                    printf("%6s %18s %18s %d\n", net_get_iface_name(item->iface), hw_addrbuf,
                           proto_addrbuf, item->etime - g_current_timestamp);
                }
            }

            return SUCCESS;
        }
		else if(!strcmp(args[0], "request"))
		{
			/* Syntax: arp request <ip> [<interface>] */
			net_iface_t *iface;
			ipv4_addr_t addr;
			net_address_t proto_addr;
			s32 ret;

			if((num_args != 2) || (strtoipv4(args[1], &addr) != SUCCESS))
				return EINVAL;

            ipv4_make_addr(addr, IPV4_PORT_NONE, &proto_addr);
            ret = ipv4_route_get_iface(&proto_addr, &iface);
            if(ret != SUCCESS)
                return ret;

			return arp_send_request(iface, &proto_addr);
		}
    }

    return EINVAL;
}


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
        u32 one = 1;

        dev->read(dev, 0, &one, &tm);

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
        u32 one = 1;
        s32 ret = rtc_time_from_str(args[0], &tm);
        if(ret != SUCCESS)
            return ret;

        return dev->write(dev, 0, &one, &tm);
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
		data[i] = console_getc();

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

	printf("kernel: %6dKB free\n"
           "  user: %6dKB free\n", kfreemem() >> 10, ufreemem() >> 10);
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
		  "arp list\n"
		  "arp request <ipv4_addr>\n"
		  "    Display or manipulate the ARP cache, or send an ARP request.\n\n"
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
		  "netif show <interface>\n"
		  "netif ipv4 <interface> <address>\n"
		  "    Display or set the protocol address associated with a network interface\n\n"
		  "raw\n"
		  "    Dump raw characters in hex format.  Ctrl-A stops.\n\n"
		  "rootfs [<partition> <type>]\n"
		  "    Set/read root partition in BIOS data area.\n\n"
		  "route list\n"
		  "route add <dest> <mask> <gateway> <metric> <interface>\n"
		  "route rm <dest> <mask> <gateway> <metric> <interface>\n"
		  "    Display or manipulate the kernel IPv4 routing table\n\n"
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
    netif

    Manipulate network interfaces
*/
MONITOR_CMD_HANDLER(netif)
{
    net_iface_t *iface;

    /*
        Syntax:
            netif show eth0
            netif ipv4 eth0 172.1.2.3
    */
    if(num_args < 2)
        return EINVAL;

    iface = net_interface_get_by_dev(args[1]);
    if(!iface)
        return ENOENT;

    if(!strcmp(args[0], "show"))
    {
        char buf[32];
        net_print_addr(net_get_proto_addr(iface), buf, 32);
        printf("%s: %s\n", net_get_iface_name(iface), buf);

        return SUCCESS;
    }
    else if(!strcmp(args[0], "ipv4"))
    {
        ipv4_addr_t ipv4addr;
        net_address_t addr;

        if(strtoipv4(args[2], &ipv4addr) != SUCCESS)
            return EINVAL;

        ipv4_make_addr(ipv4addr, IPV4_PORT_NONE, &addr);
        return net_set_proto_addr(iface, &addr);
    }

    return EINVAL;
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
        mount_ent_t *ent;
        extern mount_ent_t *g_mount_table;
        for(ent = g_mount_table; ent; ent = ent->next)
        {
            if(ent->vfs)
                printf("%-10s %-10s %s\n", ent->vfs->dev->name,
                       ent->vfs->driver->name, ent->mount_point);
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
		char c = console_getc();
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
    route

    List or manipulate the routing table
*/
MONITOR_CMD_HANDLER(route)
{
    /*
        Syntax:
            route list
            route add <net> <mask> <gateway> <metric> <interface>
            route rm <net> <mask> <gateway>
    */
    if((num_args == 1) && !strcmp(args[0], "list"))
    {
        ipv4_rt_item_t *ent = NULL;
        u8 header = 0;
        while(ipv4_route_get_entry(&ent) == SUCCESS)
        {
            char buf[3][24], flags[5];

            if(!header)
            {
                puts("Destination       Network           Gateway           Metric  Iface  Flags");
                header = 1;
            }

            flags[0] = (ent->r.flags & IPV4_ROUTE_UP) ? 'U' : ' ';
            flags[1] = (ent->r.flags & IPV4_ROUTE_GATEWAY) ? 'G' : ' ';
            flags[2] = (ent->r.flags & IPV4_ROUTE_HOST) ? 'H' : ' ';
            flags[3] = (ent->r.flags & IPV4_ROUTE_REJECT) ? '!' : ' ';
            flags[4] = '\0';

            ipv4_print_addr(&ent->r.dest, buf[0], sizeof(buf[0]));
            ipv4_print_addr(&ent->r.mask, buf[1], sizeof(buf[1]));
            ipv4_print_addr(&ent->r.gateway, buf[2], sizeof(buf[2]));
            printf("%-18s%-18s%-18s%6d  %5s  %s\n",
                   buf[0], buf[1], buf[2], ent->r.metric, net_get_iface_name(ent->r.iface), flags);
        }
        return SUCCESS;
    }
    else if(num_args >= 4)
    {
        ipv4_route_t r;
        s32 metric_, ret = 0;

        ret |= strtoipv4(args[1], &r.dest);
        ret |= strtoipv4(args[2], &r.mask);
        ret |= strtoipv4(args[3], &r.gateway);

        if((num_args == 6) && !strcmp(args[0], "add"))
        {
            metric_ = strtoul(args[4], NULL, 0);
            r.iface = net_interface_get_by_dev(args[5]);

            if(ret || (metric_ < IPV4_ROUTE_METRIC_MIN) || (metric_ > IPV4_ROUTE_METRIC_MAX)
               || !r.iface)
                return EINVAL;

            r.metric = metric_;

            r.flags = IPV4_ROUTE_UP;
            if(r.gateway != IPV4_ADDR_NONE)
                r.flags |= IPV4_ROUTE_GATEWAY;
            if(r.mask == IPV4_MASK_HOST_ONLY)
                r.flags |= IPV4_ROUTE_HOST;

            return ipv4_route_add(&r);
        }
        else if((num_args == 4) && !strcmp(args[0], "rm"))
            return ipv4_route_delete(&r);
        else
            return EINVAL;
    }
    else
        return EINVAL;
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
                u, (u32) p, (u32) p + SLAB_SIZE - 1, au, SLAB_SIZE >> au, 1 << au);
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
#include <kernel/process.h>
#include <kernel/elf.h>
#include <kernel/net/dhcp.h>
#include <kernel/net/tftp.h>
MONITOR_CMD_HANDLER(test)
{
    if(num_args < 1)
        return EINVAL;

    ku32 testnum = strtoul(args[0], NULL, 0);

    if(testnum == 2)
    {
        static const unsigned char testapp[] = {
          0x7f, 0x45, 0x4c, 0x46, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00, 0x34,
          0x00, 0x00, 0x01, 0x74, 0x01, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x02, 0x00, 0x28,
          0x00, 0x09, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
          0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x05,
          0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x02, 0x80, 0x00, 0x21, 0x02,
          0x80, 0x00, 0x21, 0x02, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x06,
          0x00, 0x00, 0x20, 0x00, 0x4e, 0x56, 0x00, 0x00, 0x48, 0xe7, 0x30, 0x34, 0x4b, 0xfa, 0x20, 0x8a,
          0x70, 0x48, 0x24, 0x6d, 0x00, 0x14, 0x26, 0x6d, 0x00, 0x0c, 0x52, 0x8a, 0x48, 0x80, 0x30, 0x40,
          0x2f, 0x08, 0x48, 0x78, 0x00, 0x02, 0x4e, 0x93, 0x10, 0x12, 0x50, 0x8f, 0x66, 0xec, 0x76, 0x0a,
          0x24, 0x6d, 0x00, 0x10, 0x74, 0x64, 0x48, 0x78, 0x00, 0x05, 0x4e, 0x92, 0x53, 0x82, 0x58, 0x8f,
          0x66, 0xf4, 0x48, 0x78, 0x00, 0x2a, 0x48, 0x78, 0x00, 0x02, 0x4e, 0x93, 0x53, 0x83, 0x50, 0x8f,
          0x66, 0xe2, 0x48, 0x78, 0x30, 0x39, 0x48, 0x78, 0x00, 0x01, 0x4e, 0x93, 0x70, 0x00, 0x4c, 0xee,
          0x2c, 0x0c, 0xff, 0xec, 0x4e, 0x5e, 0x4e, 0x75, 0x61, 0x00, 0xff, 0x9a, 0x2f, 0x00, 0x48, 0x78,
          0x00, 0x00, 0x4e, 0x40, 0x4e, 0x40, 0x4e, 0x75, 0x4e, 0x40, 0x4e, 0x75, 0x4e, 0x40, 0x4e, 0x75,
          0x4e, 0x40, 0x4e, 0x75, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,
          0x21, 0x00, 0x80, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xe8, 0x80, 0x00, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xf4,
          0x12, 0x34, 0x56, 0x78, 0x47, 0x43, 0x43, 0x3a, 0x20, 0x28, 0x47, 0x4e, 0x55, 0x29, 0x20, 0x35,
          0x2e, 0x32, 0x2e, 0x30, 0x00, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00,
          0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x2e, 0x72, 0x6f, 0x64, 0x61, 0x74, 0x61, 0x00, 0x2e, 0x64,
          0x61, 0x74, 0x61, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x72, 0x6f, 0x00, 0x2e, 0x67, 0x6f, 0x74, 0x00,
          0x2e, 0x64, 0x61, 0x74, 0x61, 0x00, 0x2e, 0x62, 0x73, 0x73, 0x00, 0x2e, 0x63, 0x6f, 0x6d, 0x6d,
          0x65, 0x6e, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x80, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x74,
          0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32,
          0x80, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x19,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x21, 0x02, 0x00, 0x00, 0x01, 0x02,
          0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03,
          0x80, 0x00, 0x21, 0x08, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x2b,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x21, 0x20, 0x00, 0x00, 0x01, 0x20,
          0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03,
          0x80, 0x00, 0x21, 0x24, 0x00, 0x00, 0x01, 0x24, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x24,
          0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
          0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x35, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
        };

        void * const addr = (void *) 0x100000;
        exe_img_t *img[10];
        pid_t pid = 0;
        s32 ret;
        u16 i;

        memcpy(addr, testapp, sizeof(testapp));

        for(i = 0; i < 10; ++i)
        {
            u32 j, count;

            ret = elf_load_exe(addr, sizeof(testapp), &(img[i]));
            if(ret != SUCCESS)
                printf("elf_load_exe() returned %u: %s\n", ret, kstrerror(ret));

            /* FIXME - there's currently no way to free an exe_img_t cleanly */

            ret = proc_create(0, 0, "testapp", img[i], NULL, NULL, 1024, 0, proc_current(), &pid);
            printf("(%d)\n", pid);

            for(j = 0, count = rand() * 5; j < count; ++j)
                cpu_nop();
        }
    }
    else if(testnum == 4)
    {
        u8 i = 0;
        vu8 * const kbdata = (vu8 *) 0xa00000;
        vu8 * const status = (vu8 *) 0xa00002;
        while((++i < 10) && (*status & 0x80))
            printf("ps2_a: %02x\n", *kbdata);
    }
    else if(testnum == 5)
    {
        /* Try to switch on keyboard LEDs */
        vu8 * const kbdata   = (vu8 *) 0xa00000;
        vu8 * const kbstatus = (vu8 *) 0xa00002;

        *kbstatus = 0;
        *kbdata = 0xed;

        // wait for command TX to complete
        while(!(*kbstatus & 0x40))
            ;

        // wait for response byte
        while(!(*kbstatus & 0x80))
            ;

        printf("response: %02x\n", *kbdata);


        *kbstatus = 0;
        *kbdata = 0x01;
        while(!(*kbstatus & 0x40))
            ;
    }
    else if(testnum == 6)
    {
        net_iface_t *iface = net_interface_get_by_dev("eth0");
        if(iface)
        {
            return dhcp_discover(iface);
        }
        else
            puts("Interface eth0 not found");
    }
    else if(testnum == 7)
    {
        net_address_t server;

        ipv4_make_addr((ipv4_addr_t) 0xac111801, IPV4_PORT_NONE, &server);   /* 172.17.24.1 */

        return tftp_read_request(&server, "test.txt");
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

	if((data = umalloc(len)) == NULL)
		return ENOMEM;

	for(data_ = data; len--;)
		*data_++ = console_getc();

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
