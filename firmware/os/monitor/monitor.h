#ifndef MONITOR_MONITOR_H_INC
#define MONITOR_MONITOR_H_INC
/*
	Monitor application

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include <platform/lambda/dfu.h>
#include <kernel/device/nvram.h>
#include <kernel/fs/mount.h>
#include <kernel/fs/vfs.h>
#include <kernel/include/console.h>
#include <kernel/include/defs.h>
#include <kernel/include/version.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/net/arp.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>
#include <klibc/include/stdlib.h>
#include <klibc/include/string.h>
#include <monitor/disasm.h>
#include <monitor/history.h>
#include <monitor/readline.h>
#include <monitor/srec.h>



#define MON_VERB_MAX_LENGTH		(16)	/* maximum length of the "verb", i.e. the first word in a
										   command line */
#define MON_MAX_ARGS			(8)		/* maximum number of arguments that may be passed to a
										   command */
#define MON_MAX_ARG_LENGTH		(255)	/* maximum length of any individual argument */

#define CMD_DUMP_DEFAULT_NUM_BYTES		(256)

#define MONITOR_CMD_HANDLER(name)    \
    s32 cmd_ ## name(ks32 num_args, s8 ** args)


struct command
{
	ks8 * const name;
	s32 (*handler)(ks32 num_args, s8 **);
};

u32 g_echo;
command_history_t *g_hist;

void monitor(void);
void monitor_main(void);
void dispatch_command(char *cmdline);

/* command handler declarations */
MONITOR_CMD_HANDLER(arp);
MONITOR_CMD_HANDLER(date);
MONITOR_CMD_HANDLER(dfu);
MONITOR_CMD_HANDLER(disassemble);
MONITOR_CMD_HANDLER(dump);
MONITOR_CMD_HANDLER(dumph);
MONITOR_CMD_HANDLER(dumpw);
MONITOR_CMD_HANDLER(echo);
MONITOR_CMD_HANDLER(fill);
MONITOR_CMD_HANDLER(fillh);
MONITOR_CMD_HANDLER(fillw);
MONITOR_CMD_HANDLER(free);
MONITOR_CMD_HANDLER(go);
MONITOR_CMD_HANDLER(help);
MONITOR_CMD_HANDLER(history);
MONITOR_CMD_HANDLER(id);
MONITOR_CMD_HANDLER(ls);
MONITOR_CMD_HANDLER(lsdev);
MONITOR_CMD_HANDLER(map);
MONITOR_CMD_HANDLER(mount);
MONITOR_CMD_HANDLER(netif);
MONITOR_CMD_HANDLER(raw);
MONITOR_CMD_HANDLER(rootfs);
MONITOR_CMD_HANDLER(route);
MONITOR_CMD_HANDLER(schedule);
MONITOR_CMD_HANDLER(serial);
MONITOR_CMD_HANDLER(slabs);
MONITOR_CMD_HANDLER(srec);
MONITOR_CMD_HANDLER(test);
MONITOR_CMD_HANDLER(upload);
MONITOR_CMD_HANDLER(write);
MONITOR_CMD_HANDLER(writeh);
MONITOR_CMD_HANDLER(writew);

#endif
