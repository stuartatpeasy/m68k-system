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
    struct rtc_time tm;

    if(num_args == 0)
    {
        char timebuf[12], datebuf[32];

        ds17485_get_time(&tm);
        time_iso8601(&tm, timebuf, sizeof(timebuf));
        date_long(&tm, datebuf, sizeof(datebuf));
        printf("%s %s\n", timebuf, datebuf);
    }
    else if(num_args == 1)
    {
        // Set the date and time.  Acceptable format:
        //      date YYYYMMDDHHMMSS
        if(rtc_time_from_str(args[0], &tm) == FAIL)
            return MON_E_SYNTAX;

        ds17485_set_time(&tm);
    }
    else
        return MON_E_SYNTAX;

    return MON_E_OK;
}


MONITOR_CMD_HANDLER(dfu)
{
	u32 len, i, ret;
	s8 *data, *endptr;

	if(num_args != 1)
		return MON_E_SYNTAX;

	len = strtoul(args[0], &endptr, 0);
	if(*endptr || !len || (len & 1))
		return MON_E_INVALID_ARG;

	if((data = kmalloc(len)) == NULL)
		return MON_E_OUT_OF_MEMORY;

    printf("Send %u bytes\n", len);
	for(i = 0; i < len; i++)
		data[i] = duarta_getc();

	if((ret = dfu((ku16 *) data, len)))
	{
		printf("dfu() returned %i\n", ret);
	}

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(disassemble)
{
	u32 num_bytes, start;
	s8 *endptr;
	u16 *addr;
	s8 line[80], instr_printed;

	if(!num_args || (num_args > 2))
		return MON_E_SYNTAX;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr || (start & 1))
		return MON_E_INVALID_ARG;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || (num_bytes < 2) || (num_bytes & 1))
			return MON_E_INVALID_ARG;
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

	return MON_E_OK;
}


s32 dump_mem(ks32 start, ks32 num_bytes, ks8 word_size)
{
	ku8 line_length = 16;
	u8 x, y;
	s8 *line;
	u32 offset, data;

	if((word_size != 4) && (word_size != 2) && (word_size != 1))
		return MON_E_INTERNAL_ERROR;

	if(!(line = (u8 *) kmalloc(line_length)))
		return MON_E_OUT_OF_MEMORY;

	for(offset = 0; offset < num_bytes; offset += line_length)
	{
		printf("%06x: ", start + offset);
		for(x = 0; x < line_length; x += word_size)
		{
			if((offset + x) < num_bytes)
				switch(word_size)
				{
				case 1:
					data = *((u8 *) (start + offset + x));
					printf("%02x ", (u8) data);
					line[x] = data;
					break;

				case 2:
					data = *((u16 *) (start + offset + x));
					printf("%04x ", (u16) data);
					((u16 *) line)[x >> 1] = (u16) data;
					break;

				case 4:
					data = *((u32 *) (start + offset + x));
					printf("%08x ", data);
					((u32 *) line)[x >> 2] = data;
					break;
				}
			else
				for(y = (word_size << 1) + 1; y--;)
					putchar(' ');
		}
		putchar(' ');

		for(x = 0; x < line_length; ++x)
		{
			if((offset + x) < num_bytes)
				putchar(isprint(line[x]) ? line[x] : '.');
			else
				putchar(' ');
		}
		puts("");
	}

	kfree(line);

	return MON_E_OK;
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
		return MON_E_SYNTAX;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || (num_bytes < 1))
			return MON_E_INVALID_ARG;
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
		return MON_E_SYNTAX;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	if(start & 1)
		return MON_E_INVALID_ARG;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || !num_bytes || (num_bytes & 1))
			return MON_E_INVALID_ARG;
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
		return MON_E_SYNTAX;

	start = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	if(start & 3)
		return MON_E_INVALID_ARG;

	if(num_args == 2)
	{
		num_bytes = strtoul(args[1], &endptr, 0);
		if(*endptr || !num_bytes || (num_bytes & 3))
			return MON_E_INVALID_ARG;
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
    u32 i;
    for(i = 0; i < num_args; ++i)
    {
        if(i)
            putchar(' ');
        put(args[i]);
    }

    if(num_args)
        puts("");

    return MON_E_OK;
}


MONITOR_CMD_HANDLER(fill)
{
	u32 count, data;
	u8 *start;
	s8 *endptr;

	if(num_args != 3)
		return MON_E_SYNTAX;

	start = (u8 *) strtoul(args[0], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr | !count)
		return MON_E_INVALID_ARG;

	data = strtoul(args[2], &endptr, 0);
	if((*endptr) || (data > 0xff))
		return MON_E_INVALID_ARG;

	while(count--)
		*((u8 *) start++) = (u8) data;

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(fillh)
{
	u32 count, data;
	u16 *start;
	s8 *endptr;

	if(num_args != 3)
		return MON_E_SYNTAX;

	start = (u16 *) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) start & 0x1))
		return MON_E_INVALID_ARG;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr || !count)
		return MON_E_INVALID_ARG;

	data = strtoul(args[2], &endptr, 0);
	if((*endptr) || (data > 0xffff))
		return MON_E_INVALID_ARG;

	while(count--)
		*start++ = (u16) data;

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(fillw)
{
	u32 *start, count, data;
	s8 *endptr;

	if(num_args != 3)
		return MON_E_SYNTAX;

	start = (u32 *) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) start & 0x3))
		return MON_E_INVALID_ARG;

	count = strtoul(args[1], &endptr, 0);
	if(*endptr | !count)
		return MON_E_INVALID_ARG;

	data = strtoul(args[2], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	while(count--)
		*start++ = data;

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(free)
{
	printf("%d bytes free\n", kfreemem());
	return MON_E_OK;
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
		return MON_E_SYNTAX;

	addr = (void (*)()) strtoul(args[0], &endptr, 0);
	if(*endptr || ((u32) addr & 1))
		return MON_E_INVALID_ARG;

	addr();

	return MON_E_OK;
}


/*
	help

	Display helpful text.
*/
MONITOR_CMD_HANDLER(help)
{
	puts("Available commands (all can be abbreviated):\n\n"
          "date [<newdate>]\n"
          "    If no argument is supplied, print the current date and time.  If date is specified\n"
          "    in YYYYMMDDHHMMSS format, set the RTC date and time accordingly.\n\n"
		  "dfu <size>\n"
		  "    Receive <size> bytes and re-flash the firmware ROMs with this data.  <size> must\n"
		  "    be an even number\n\n"
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
		  "id\n"
		  "    Display device identity\n\n"
		  "raw\n"
		  "    Dump raw characters in hex format.  Ctrl-A stops.\n\n"
		  "schedule\n"
		  "    Start task scheduler\n\n"
		  "serial\n"
		  "    Configure serial line discipline:\n"
		  "        serial echo off - disable character echo\n"
		  "        serial echo on  - enable character echo\n\n"
		  "srec\n"
		  "    Start the upload of an S-record file\n\n"
		  "upload <count>\n"
		  "    Receive <count> bytes and place them in memory\n\n"
		  "write[h|w] <address> <data>\n"
		  "    Write <data> to to memory at <address>\n\n"
		);
	return MON_E_OK;
}


/*
    id

    Display device identity
*/
MONITOR_CMD_HANDLER(id)
{
    puts(OS_NAME " v" OS_VERSION_STR " on " CPU_NAME ", build date " __DATE__ " " __TIME__);
    return MON_E_OK;
}


/*
	raw

	Hex-dump received characters until ctrl-A (0x01) is received.
*/
MONITOR_CMD_HANDLER(raw)
{
	puts("Dumping raw output.  Use ctrl-A to stop.\n");
	for(;;)
	{
		char c = duarta_getc();
		printf("0x%02x ", c);

		if(c == 0x01 /* ctrl-A */)
			break;
	}

	puts("\n");
	return MON_E_OK;
}


/*
    sched

    Start scheduler
*/
MONITOR_CMD_HANDLER(schedule)
{
    duart_start_counter();
    return MON_E_OK;
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
                return MON_E_OK;
            }
            else if(!strcmp(args[1], "off"))
            {
                g_echo = 0;
                return MON_E_OK;
            }
        }
    }

    return MON_E_SYNTAX;
}


/*
	srec

	Receive data in S-record format and put it in memory somwhere
*/
MONITOR_CMD_HANDLER(srec)
{
	if(num_args)
		return MON_E_SYNTAX;

	struct srec_data s;

	if(srec(&s))
	{
		printf("S-record error at line %u: %s\n", s.line, srec_strerror(s.error));
		return MON_E_OK;	/* FIXME: find a way of returning an already-reported error */
	}

	printf("Uploaded %u bytes at address %p\nStart address %x\n",
				s.data_len, s.data, s.start_address);

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(upload)
{
	u32 len;
	s8 *data, *data_, *endptr;

	if(num_args != 1)
		return MON_E_SYNTAX;

	len = strtoul(args[0], &endptr, 0);
	if(*endptr || !len)
		return MON_E_INVALID_ARG;

	if((data = kmalloc(len)) == NULL)
		return MON_E_OUT_OF_MEMORY;

	for(data_ = data; len--;)
		*data_++ = duarta_getc();

	printf("Uploaded %li bytes at %p\n", data_ - data, data);

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(write)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return MON_E_SYNTAX;

	addr = strtoul(args[0], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	data = strtoul(args[1], &endptr, 0);
	if((*endptr) || (data > 0xff))
		return MON_E_INVALID_ARG;

	*((u8 *) addr) = (u8) data;

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(writeh)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return MON_E_SYNTAX;

	addr = strtoul(args[0], &endptr, 0);
	if((*endptr) || (addr & 0x1))
		return MON_E_INVALID_ARG;

	data = strtoul(args[1], &endptr, 0);
	if((*endptr) || (data > 0xffff))
		return MON_E_INVALID_ARG;

	*((u16 *) addr) = (u16) data;

	return MON_E_OK;
}


MONITOR_CMD_HANDLER(writew)
{
	u32 addr, data;
	s8 *endptr;

	if(num_args != 2)
		return MON_E_SYNTAX;

	addr = strtoul(args[0], &endptr, 0);
	if((*endptr) || (addr & 0x3))
		return MON_E_INVALID_ARG;

	data = strtoul(args[1], &endptr, 0);
	if(*endptr)
		return MON_E_INVALID_ARG;

	*((u32 *) addr) = data;

	return MON_E_OK;
}

