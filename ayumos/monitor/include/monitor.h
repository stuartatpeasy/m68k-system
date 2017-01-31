#ifndef MONITOR_MONITOR_H_INC
#define MONITOR_MONITOR_H_INC
/*
	Monitor application

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include <platform/lambda/include/dfu.h>
#include <kernel/include/fs/mount.h>
#include <kernel/include/console.h>
#include <kernel/include/defs.h>
#include <kernel/include/device/nvram.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/ksym.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/net/arp.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/version.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>
#include <klibc/include/stdlib.h>
#include <klibc/include/string.h>
#include <monitor/include/disasm.h>
#include <monitor/include/history.h>
#include <monitor/include/readline.h>
#include <monitor/include/srec.h>


#define MON_VERB_MAX_LENGTH		(16)	/* maximum length of the "verb", i.e. the first word in a
										   command line */
#define MON_MAX_ARGS			(8)		/* maximum number of arguments that may be passed to a
										   command */
#define MON_MAX_ARG_LENGTH		(255)	/* maximum length of any individual argument */

#define CMD_DUMP_DEFAULT_NUM_BYTES		(256)

#define MONITOR_CMD_HANDLER(name)    \
    s32 cmd_ ## name(ks32 num_args, s8 ** args)


/* Flags that can be used with monitor_parse_arg() to validate arg values */
#define MPA_NONE            (0)         /* Placeholder meaning "skip value validation"          */
#define MPA_BYTE            (1 << 0)    /* Fail if val > 0xff                                   */
#define MPA_HWORD           (1 << 1)    /* Fail if val > 0xffff                                 */
#define MPA_ALIGN_HWORD     (1 << 2)    /* Fail if (val & 1) != 0                               */
#define MPA_ALIGN_WORD      (1 << 3)    /* Fail if (val & 3) != 0                               */
#define MPA_NOT_ZERO        (1 << 4)    /* Fail if val == 0                                     */
#define MPA_AT_LEAST_2      (1 << 5)    /* Fail if val < 2                                      */

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
s32 monitor_parse_arg(const char *arg, unsigned int *val, unsigned int flags);

/* command handler declarations */

#ifdef WITH_MASS_STORAGE
MONITOR_CMD_HANDLER(ls);
MONITOR_CMD_HANDLER(mount);
MONITOR_CMD_HANDLER(rootfs);
#endif /* WITH_MASS_STORAGE */

#ifdef WITH_NETWORKING
MONITOR_CMD_HANDLER(arp);
MONITOR_CMD_HANDLER(netif);
MONITOR_CMD_HANDLER(route);
#endif /* WITH_NETWORKING */

#ifdef WITH_RTC
MONITOR_CMD_HANDLER(date);
#endif /* WITH_RTC */

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
MONITOR_CMD_HANDLER(lsdev);
MONITOR_CMD_HANDLER(map);
MONITOR_CMD_HANDLER(raw);
MONITOR_CMD_HANDLER(schedule);
MONITOR_CMD_HANDLER(serial);
MONITOR_CMD_HANDLER(slabs);
MONITOR_CMD_HANDLER(srec);
MONITOR_CMD_HANDLER(symbol);
MONITOR_CMD_HANDLER(test);
MONITOR_CMD_HANDLER(upload);
MONITOR_CMD_HANDLER(write);
MONITOR_CMD_HANDLER(writeh);
MONITOR_CMD_HANDLER(writew);

#endif
