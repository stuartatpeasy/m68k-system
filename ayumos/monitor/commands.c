/*
    Monitor application command handlers

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2011.
*/

#include <monitor/include/monitor.h>

/* lsdev command output-formatting flags */
#define LSDEV_LONG_FORMAT       BIT(0)


/*
    arp

    Work with the ARP cache
*/
#ifdef WITH_NETWORKING
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

                    net_address_print(&item->hw_addr, hw_addrbuf, sizeof(hw_addrbuf));
                    net_address_print(&item->proto_addr, proto_addrbuf, sizeof(proto_addrbuf));

                    printf("%6s %18s %18s %d\n", net_get_iface_name(item->iface), hw_addrbuf,
                           proto_addrbuf, item->etime - g_current_timestamp);
                }
            }

            return SUCCESS;
        }
        else if(!strcmp(args[0], "request"))
        {
            /* Syntax: arp request <ip> */
            ipv4_addr_t addr;
            net_address_t proto_addr;

            if((num_args != 2) || (strtoipv4(args[1], &addr) != SUCCESS))
                return -EINVAL;

            ipv4_make_addr(addr, IPV4_PORT_NONE, &proto_addr);

            return arp_send_request(&proto_addr);
        }
    }

    return -EINVAL;
}
#endif /* WITH_NETWORKING */


/*
    date

    Get/set the date
*/
#ifdef WITH_RTC
MONITOR_CMD_HANDLER(date)
{
    rtc_time_t tm;
    dev_t *dev;

    if(num_args > 1)
        return -EINVAL;

    /* Find an RTC */
    dev = dev_find("rtc0");
    if(dev == NULL)
        return -ENODEV;

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
        return -EINVAL;

    return SUCCESS;
}
#endif /* WITH_RTC */


/*
    dfu <len> <checksum>

    Re-flash boot ROM
*/
MONITOR_CMD_HANDLER(dfu)
{
    u32 len, buffer_len, cksum_sent, cksum_calculated, i;
    s32 ret;
    s8 *data;

    if(num_args != 2)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &len, MPA_NOT_ZERO);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &cksum_sent, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    /*
        The dfu() function requires an even number of bytes, so - if the new firmware image is an
        odd number of bytes in length, we allocate an extra byte to the data buffer and add a
        padding byte (0x00).
    */
    buffer_len = (len & 1) ? len + 1 : len;

    if((data = umalloc(buffer_len)) == NULL)
        return -ENOMEM;

    printf("Send %u bytes\n", len);
    for(i = 0; i < len; i++)
        data[i] = console_getc();

    if(len & 1)
        data[i] = 0x00;     /* Add padding byte - see above */

    cksum_calculated = fletcher16(data, len);
    if(cksum_calculated != cksum_sent)
        return -ECKSUM;

    if((ret = dfu((ku16 *) data, buffer_len)))
        printf("Firmware update failed: %s\n", kstrerror(-ret));

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
    s32 ret;
    u16 *addr;
    s8 line[80], instr_printed;

    if(!num_args || (num_args > 2))
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &start, MPA_ALIGN_HWORD);
    if(ret != SUCCESS)
        return ret;

    if(num_args == 2)
    {
        ret = monitor_parse_arg(args[1], &num_bytes, MPA_ALIGN_HWORD | MPA_AT_LEAST_2);
        if(ret != SUCCESS)
            return ret;
    }
    else num_bytes = 256;

    for(addr = (u16 *) start; addr < (u16 *) (start + num_bytes);)
    {
        u16 *addr_ = addr;

        disassemble(mt68010, &addr_, line);
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
    s32 ret;

    if(!num_args || (num_args > 2))
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &start, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    if(num_args == 2)
    {
        ret = monitor_parse_arg(args[1], &num_bytes, MPA_NONE);
        if(ret != SUCCESS)
            return ret;
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
    s32 ret;

    if(!num_args || (num_args > 2))
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &start, MPA_ALIGN_HWORD);
    if(ret != SUCCESS)
        return ret;

    if(num_args == 2)
    {
        ret = monitor_parse_arg(args[1], &num_bytes, MPA_ALIGN_HWORD);
        if(ret != SUCCESS)
            return ret;
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
    s32 ret;

    if(!num_args || (num_args > 2))
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &start, MPA_ALIGN_WORD);
    if(ret != SUCCESS)
        return ret;

    if(num_args == 2)
    {
        ret = monitor_parse_arg(args[1], &num_bytes, MPA_ALIGN_WORD);
        if(ret != SUCCESS)
            return ret;
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


/*
    fill <start> <count> <val>

    Fill <count> bytes, starting at <start>, with the value <val>.
*/
MONITOR_CMD_HANDLER(fill)
{
    u32 count, data;
    u8 *start;
    s32 ret;

    if(num_args != 3)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], (unsigned int *) &start, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &count, MPA_NOT_ZERO);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[2], &data, MPA_BYTE);
    if(ret != SUCCESS)
        return ret;

    while(count--)
        *((u8 *) start++) = (u8) data;

    return SUCCESS;
}


/*
    fillh <start> <count> <val>

    Fill <count> half-words, starting at <start>, with the value <val>.  <start> must be half-word
    aligned.
*/
MONITOR_CMD_HANDLER(fillh)
{
    u32 count, data;
    u16 *start;
    s32 ret;

    if(num_args != 3)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], (unsigned int *) &start, MPA_ALIGN_HWORD);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &count, MPA_NOT_ZERO);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[2], &data, MPA_HWORD);
    if(ret != SUCCESS)
        return ret;

    while(count--)
        *start++ = (u16) data;

    return SUCCESS;
}


/*
    fillw <start> <count> <val>

    Fill <count> words, starting at <start>, with the value <val>.  <start> must be word-aligned.
*/
MONITOR_CMD_HANDLER(fillw)
{
    u32 *start, count, data;
    s32 ret;

    if(num_args != 3)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], (unsigned int *) &start, MPA_ALIGN_WORD);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &count, MPA_NOT_ZERO);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[2], &data, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

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
    s32 ret;
    void (*addr)() = NULL;

    if(num_args != 1)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], (unsigned int *) addr, MPA_ALIGN_HWORD);
    if(ret != SUCCESS)
        return ret;

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
#ifdef WITH_NETWORKING
          "arp list\n"
          "arp request <ipv4_addr>\n"
          "    Display or manipulate the ARP cache, or send an ARP request.\n\n"
#endif
#ifdef WITH_RTC
          "date [<newdate>]\n"
          "    If no argument is supplied, print the current date and time.  If date is specified\n"
          "    in YYYYMMDDHHMMSS format, set the RTC date and time accordingly.\n\n"
#endif
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
#ifdef WITH_MASS_STORAGE
          "ls [<path>]\n"
          "    List directory contents\n\n"
#endif
          "lsdev\n"
          "    List devices\n\n"
          "map\n"
          "    Display memory map\n\n"
#ifdef WITH_MASS_STORAGE
          "mount [<dev> <fstype> <mountpoint>]\n"
          "    With no arguments, list current mounts.  With arguments, mount block device <dev>\n"
          "    containing a file system of type <fstype> at <mountpoint>\n\n"
#endif
#ifdef WITH_NETWORKING
          "netif show <interface>\n"
          "netif ipv4 <interface> <address>\n"
          "    Display or set the protocol address associated with a network interface\n\n"
#endif
          "raw\n"
          "    Dump raw characters in hex format.  Ctrl-A stops.\n\n"
#ifdef WITH_MASS_STORAGE
          "rootfs [<partition> <type>]\n"
          "    Set/read root partition in BIOS data area.\n\n"
#endif
#ifdef WITH_NETWORKING
          "route list\n"
          "route add <dest> <mask> <gateway> <metric> <interface>\n"
          "route rm <dest> <mask> <gateway> <metric> <interface>\n"
          "    Display or manipulate the kernel IPv4 routing table\n\n"
#endif
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
          "symbol [-v] <name>\n"
          "    Display the memory location of symbol <name>, if available.  If the -v (verbose)\n"
          "    option is given, display symbol type information.\n\n"
          "upload <count>\n"
          "    Receive <count> bytes and place them in memory\n\n"
          "write[h|w] <address> <data>\n"
          "    Write <data> to to memory at <address>\n\n"
        );
    return SUCCESS;
}


/*
    history

    Display command history.
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

    Display device identity (OS name, version, CPU architecture, build date/time).
*/
MONITOR_CMD_HANDLER(id)
{
    UNUSED(num_args);
    UNUSED(args);

    puts(OS_NAME " v" OS_VERSION_STR " on " CPU_NAME ", build date " __DATE__ " " __TIME__);
    return SUCCESS;
}


/*
    ls <path>

    List directory contents.
*/
#ifdef WITH_MASS_STORAGE
MONITOR_CMD_HANDLER(ls)
{

    /* For now we require a path arg */
    if(num_args == 1)
    {
        char perms[11];
        vfs_t *vfs;
        fs_node_t *node;
        s32 ret;

        perms[10] = '\0';

        ret = path_open(args[0], &vfs, &node);

        if(ret != SUCCESS)
        {
            puts(kstrerror(-ret));
            return SUCCESS;
        }

        if(node->type == FSNODE_TYPE_DIR)
        {
            /* Iterate directory */
            vfs_dir_ctx_t *ctx;

            ret = vfs_open_dir(vfs, node, &ctx);
            if(ret != SUCCESS)
            {
                puts(kstrerror(-ret));
                fs_node_free(node);
                return SUCCESS;
            }

            while((ret = vfs_read_dir(ctx, NULL, node)) != -ENOENT)
            {
                printf("%9d %s %9d %s\n", node->first_block, fs_node_perm_str(node, perms),
                       node->size, node->name);
            }

            if(ret != -ENOENT)
                puts(kstrerror(-ret));

            vfs_close_dir(ctx);
        }
        else
        {
            /* Print single file */
            printf("%9d %s %9d %s\n", node->first_block, fs_node_perm_str(node, perms),
                   node->size, node->name);
        }

        fs_node_free(node);
    }
    else
        return -EINVAL;

    return SUCCESS;
}
#endif /* WITH_MASS_STORAGE */


/*
    lsdev [-l]

    List devices.  If the "-l" option is supplied, list extra information about each device.
*/
MONITOR_CMD_HANDLER(lsdev)
{
    dev_t *dev;
    u32 flags = 0;
    UNUSED(num_args);
    UNUSED(args);

    if(args)
    {
        int i;
        for(i = 0; i < num_args; ++i)
        {
            if(!strcmp(args[i], "-l"))
                flags |= LSDEV_LONG_FORMAT;
        }
    }

    for(dev = dev_get_root(); (dev = dev_get_next(dev)) != NULL;)
    {
        if(flags & LSDEV_LONG_FORMAT)
        {
            printf("%08x %c %12s %s\n", (u32) dev->base_addr, dev_get_type_char(dev), dev->name,
                    dev->human_name);
        }
        else
            puts(dev->name);
    }

    return SUCCESS;
}


/*
    map

    Display memory map.
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
#ifdef WITH_MASS_STORAGE
MONITOR_CMD_HANDLER(mount)
{
    UNUSED(num_args);
    UNUSED(args);

    return -ENOSYS;
#if 0
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
        return mount_add(args[2], vfs_get_driver_by_name(args[1]), dev_find(args[0]));
    }
    else
        return -EINVAL;

    return SUCCESS;
#endif
}
#endif /* WITH_MASS_STORAGE */


/*
    netif <"show"|"ipv4"> <interface> [<args...>]

    Manipulate network interfaces.  Args are described in greater detail below.
*/
#ifdef WITH_NETWORKING
MONITOR_CMD_HANDLER(netif)
{
    net_iface_t *iface;

    /*
        Syntax:
            netif show eth0
            netif ipv4 eth0 172.1.2.3
    */
    if(num_args < 2)
        return -EINVAL;

    iface = net_interface_get_by_dev(args[1]);
    if(!iface)
        return -ENOENT;

    if(!strcmp(args[0], "show"))
    {
        char buf[32];
        const net_iface_stats_t *stats;

        net_address_print(net_interface_get_proto_addr(iface), buf, 32);
        stats = net_interface_get_stats(iface);

        printf("%s: %s\n"
               "RX packets: %u    bytes: %u    checksum err: %u    dropped: %u\n"
               "TX packets: %u    bytes: %u\n"
               , net_get_iface_name(iface), buf,
               stats->rx_packets, stats->rx_bytes, stats->rx_checksum_err, stats->rx_dropped,
               stats->tx_packets, stats->tx_bytes);

        return SUCCESS;
    }
    else if(!strcmp(args[0], "ipv4"))
    {
        ipv4_addr_t ipv4addr;
        net_address_t addr;

        if(strtoipv4(args[2], &ipv4addr) != SUCCESS)
            return -EINVAL;

        ipv4_make_addr(ipv4addr, IPV4_PORT_NONE, &addr);
        return net_interface_set_proto_addr(iface, &addr);
    }

    return -EINVAL;
}
#endif /* WITH_NETWORKING */


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
#ifdef WITH_MASS_STORAGE
MONITOR_CMD_HANDLER(rootfs)
{
    nvram_bpb_t bpb;
    s32 ret;

    if((num_args != 0) && (num_args != 2))
        return -EINVAL;

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
            return -EINVAL;

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
#endif /* WITH_MASS_STORAGE */


/*
    route <args...>

    List or manipulate the routing table.  Args are described below.
*/
#ifdef WITH_NETWORKING
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
            net_address_t dest, mask, gateway;

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

            ipv4_make_addr(ent->r.dest, IPV4_PORT_NONE, &dest);
            ipv4_make_addr(ent->r.mask, IPV4_PORT_NONE, &mask);
            ipv4_make_addr(ent->r.gateway, IPV4_PORT_NONE, &gateway);

            ipv4_print_addr(&dest, buf[0], sizeof(buf[0]));
            ipv4_print_addr(&mask, buf[1], sizeof(buf[1]));
            ipv4_print_addr(&gateway, buf[2], sizeof(buf[2]));
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
            ks32 ret2 = monitor_parse_arg(args[4], (unsigned int *) &metric_, MPA_NONE);
            r.iface = net_interface_get_by_dev(args[5]);

            if(ret || ret2 || (metric_ < IPV4_ROUTE_METRIC_MIN) || (metric_ > IPV4_ROUTE_METRIC_MAX)
               || !r.iface)
                return -EINVAL;

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
            return -EINVAL;
    }
    else
        return -EINVAL;
}
#endif /* WITH_NETWORKING */


/*
    sched

    Start the process scheduler.
*/
MONITOR_CMD_HANDLER(schedule)
{
    UNUSED(num_args);
    UNUSED(args);

    plat_start_quantum();
    return SUCCESS;
}


/*
    serial <"echo" <"on"|"off">>

    Configure serial line discipline.  Currently only supports switching echo on and off.
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

    return -EINVAL;
}


/*
    slabs

    Display slab allocation status
*/
MONITOR_CMD_HANDLER(slabs)
{
    u16 radix;
    UNUSED(num_args);
    UNUSED(args);

    for(radix = SLAB_MIN_RADIX; radix <= SLAB_MAX_RADIX; ++radix)
    {
        u32 total, free;

        if(slab_get_stats(radix, &total, &free) == SUCCESS)
        {
            printf("%2u: %u/%u\n", 1 << radix, total - free, total);
        }
    }

    return SUCCESS;
}


/*
    srec

    Receive data in S-record format and put it in user memory somewhere.  Display the address of the
    received data.
*/
MONITOR_CMD_HANDLER(srec)
{
    UNUSED(args);

    if(num_args)
        return -EINVAL;

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
    symbol [-v] <sym>

    Display the address of the kernel symbol <sym>.  If the "-v" option is supplied, display
    detailed information.
*/
MONITOR_CMD_HANDLER(symbol)
{
    u8 verbose;
    char *sym;
    symentry_t *ent;
    s32 ret;

    if(num_args == 2)
    {
        if(strcmp(args[0], "-v"))
            return -EINVAL;

        verbose = 1;
        sym = args[1];
    }
    else if(num_args == 1)
    {
        verbose = 0;
        sym = args[0];
    }
    else
        return -EINVAL;

    ret = ksym_find_by_name(sym, &ent);
    if(ret != SUCCESS)
        return ret;

    if(!verbose)
        printf("%p\n", ent->addr);
    else
        printf("%p    %s    %s\n", ent->addr, sym, ksym_get_description(ent->type));

    return SUCCESS;
}


/*
    test

    Used to trigger a test of some sort
*/
#include <kernel/include/elf.h>
#include <kernel/include/process.h>
#include <kernel/include/tick.h>
#include <kernel/include/net/dhcp.h>
#include <kernel/include/net/tftp.h>

MONITOR_CMD_HANDLER(test)
{
    u32 testnum;
    s32 ret;

    if(num_args < 1)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &testnum, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    if(testnum == 1)
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
            ret = elf_load_exe(addr, sizeof(testapp), &(img[i]));
            if(ret != SUCCESS)
                printf("elf_load_exe() returned %u: %s\n", ret, kstrerror(-ret));

            /* FIXME - there's currently no way to free an exe_img_t cleanly */

            ret = proc_create(0, 0, "testapp", img[i], NULL, NULL, 1024, 0, PROC_DEFAULT_WD,
                              proc_current(), &pid);
            printf("(%d)\n", pid);
        }
    }
#ifdef WITH_NETWORKING
    else if(testnum == 2)
    {
        net_iface_t *iface = net_interface_get_by_dev("eth0");
        if(iface)
        {
            return dhcp_discover(iface);
        }
        else
            puts("Interface eth0 not found");
    }
    else if(testnum == 3)
    {
        net_address_t server;

        ipv4_make_addr((ipv4_addr_t) 0xac111801, IPV4_PORT_NONE, &server);   /* 172.17.24.1 */

        return tftp_read_request(&server, "test.txt");
    }
#endif /* WITH_NETWORKING */
    else if(testnum == 4)
    {
        file_handle_t *fh;

        ret = file_open(args[1], O_RD, &fh);
        if(ret == SUCCESS)
            file_close(fh);

        return ret;
    }

    return SUCCESS;
}


/*
    upload <len>

    Allocate memory in user space, receive <len> bytes, store them in the allocated region.  Return
    the address of the allocated memory.
*/
MONITOR_CMD_HANDLER(upload)
{
    u32 len;
    s32 ret;
    s8 *data, *data_;

    if(num_args != 1)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &len, MPA_NOT_ZERO);
    if(ret)
        return ret;

    if((data = umalloc(len)) == NULL)
        return -ENOMEM;

    for(data_ = data; len--;)
        *data_++ = console_getc();

    printf("Uploaded %li bytes at %p\n", data_ - data, data);

    return SUCCESS;
}


/*
    write <addr> <val>

    Write the byte <val> to address <addr>.
*/
MONITOR_CMD_HANDLER(write)
{
    u32 addr, data;
    s32 ret;

    if(num_args != 2)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &addr, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &data, MPA_BYTE);
    if(ret != SUCCESS)
        return ret;

    *((u8 *) addr) = (u8) data;

    return SUCCESS;
}


/*
    writeh <addr> <val>

    Write the half-word <val> to the address <addr>.  <addr> must be half-word aligned.
*/
MONITOR_CMD_HANDLER(writeh)
{
    u32 addr, data;
    s32 ret;

    if(num_args != 2)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &addr, MPA_ALIGN_HWORD);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &data, MPA_HWORD);
    if(ret != SUCCESS)
        return ret;

    *((u16 *) addr) = (u16) data;

    return SUCCESS;
}


/*
    writew <addr> <val>

    Write the word <val> to the address <addr>.  <addr> must be word-aligned.
*/
MONITOR_CMD_HANDLER(writew)
{
    u32 addr, data;
    s32 ret;

    if(num_args != 2)
        return -EINVAL;

    ret = monitor_parse_arg(args[0], &addr, MPA_ALIGN_WORD);
    if(ret != SUCCESS)
        return ret;

    ret = monitor_parse_arg(args[1], &data, MPA_NONE);
    if(ret != SUCCESS)
        return ret;

    *((u32 *) addr) = data;

    return SUCCESS;
}
