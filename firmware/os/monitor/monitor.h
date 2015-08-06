#ifndef MONITOR_MONITOR_H_INC
#define MONITOR_MONITOR_H_INC
/*
	Monitor application

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include <dfu.h>
#include <ds17485.h>
#include <include/defs.h>
#include <include/version.h>
#include <memory/kmalloc.h>
#include <memory/ramdetect.h>
#include <memory/slab.h>
#include <monitor/disasm.h>
#include <monitor/history.h>
#include <monitor/readline.h>
#include <monitor/srec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MON_VERB_MAX_LENGTH		(16)	/* maximum length of the "verb", i.e. the first word in a
										   command line */
#define MON_MAX_ARGS			(8)		/* maximum number of arguments that may be passed to a
										   command */
#define MON_MAX_ARG_LENGTH		(255)	/* maximum length of any individual argument */

#define CMD_DUMP_DEFAULT_NUM_BYTES		(256)

#define MONITOR_CMD_HANDLER(name)    \
    s32 cmd_ ## name(ks32 num_args, s8 ** args)

/*
	Return / error codes for commands
*/
#define MON_E_OK				(0)
#define MON_E_SYNTAX			(1)
#define MON_E_INVALID_ARG		(2)
#define MON_E_NOT_IMPLEMENTED	(3)
#define MON_E_INTERNAL_ERROR	(4)
#define MON_E_OUT_OF_MEMORY		(5)


struct command
{
	ks8 * const name;
	s32 (*handler)(ks32 num_args, s8 **);
};

u32 g_echo;

void monitor(void);
void monitor_main(void);
void dispatch_command(const char *cmdline);

/* command handler declarations */
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
MONITOR_CMD_HANDLER(map);
MONITOR_CMD_HANDLER(raw);
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
