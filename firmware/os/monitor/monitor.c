/*
	Monitor application

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "monitor.h"
#include "kutil/kutil.h"
#include "memory/kmalloc.h"

#define CMD_MAX_LEN (255)
#define HISTORY_LEN (32)

ks8 * const g_prompt = "$ ";
ks8 * g_history[HISTORY_LEN];
u32 g_history_ptr;
u32 g_echo;

const struct command g_commands[] =
{
    {"date",            cmd_date},
	{"dfu",				cmd_dfu},
	{"disassemble",		cmd_disassemble},
	{"dump",			cmd_dump},
	{"dumph",			cmd_dumph},
	{"dumpw",			cmd_dumpw},
	{"echo",            cmd_echo},
	{"fill",			cmd_fill},
	{"fillh",			cmd_fillh},
	{"fillw",			cmd_fillw},
	{"free",			cmd_free},
	{"help",			cmd_help},
	{"id",              cmd_id},
	{"go",				cmd_go},
	{"raw",				cmd_raw},
	{"schedule",        cmd_schedule},
	{"serial",          cmd_serial},
	{"srec",			cmd_srec},
	{"upload",			cmd_upload},
	{"write",			cmd_write},
	{"writeh",			cmd_writeh},
	{"writew",			cmd_writew},

	{NULL, NULL}
};


void monitor(void)
{
blah:
	monitor_main();
	goto blah;
}


void monitor_main(void)
{
	char buffer[CMD_MAX_LEN + 1];

    g_echo = 1;
	g_history_ptr = 0;
	kputchar('\n');

	for(buffer[CMD_MAX_LEN] = '\0'; ;)
	{
		kput(g_prompt);
		readline(buffer, CMD_MAX_LEN, g_echo);

		if(kstrlen(buffer))
		{
			dispatch_command(buffer);
		}
	}
}


void dispatch_command(const char *cmdline)
{
	unsigned char c = 0, num_args = 0, cmd_len;
	char command[MON_VERB_MAX_LENGTH + 1];
	s8 *args[MON_MAX_ARGS + 1];

	/* trim leading space */
	while((*cmdline == ' ') || (*cmdline == '\t'))
		++cmdline;

	for(c = 0; *cmdline && (c < ((sizeof(command) / sizeof(unsigned char)) - 1)) && (*cmdline != ' ');
		command[c++] = *cmdline++) ;
	command[c] = '\0';

	cmd_len = kstrlen(command);
	if(!cmd_len)
		return;

	const struct command *p, *pcommand = NULL;
	for(p = g_commands; p->name; ++p)
		if(kstrstr(p->name, command) == p->name)
		{
			if(pcommand)
			{
				kputs("Ambiguous command");
				return;
			}
			else
			{
				pcommand = p;
				if(cmd_len == kstrlen(p->name))
					break;	/* exact match */
			}
		}

	if(!pcommand)
	{
		printf("Unrecognised command '%s'\n", command);
		return;
	}

	/* extract arguments */
	for(; *cmdline;)
	{
		const char *argptr;

		/* step over whitespace */
		while((*cmdline == '\n') || (*cmdline == '\r') || (*cmdline == ' ') || (*cmdline == '\t'))
			++cmdline;

		if(*cmdline)
		{
			if(num_args >= MON_MAX_ARGS)
			{
				kputs("Too many arguments");
				return;
			}

			/* read argument */
			argptr = cmdline;

			while(*cmdline && (*cmdline != '\n') && (*cmdline != '\r') && (*cmdline != ' ') && (*cmdline != '\t'))
				++cmdline;

			if((cmdline - argptr) > MON_MAX_ARG_LENGTH)
			{
				printf("Argument %d too long\n", num_args + 1);
				return;
			}

			if(!(args[num_args] = kmalloc(cmdline - argptr + 1)))
			{
				kputs("Out of memory");
				return;
			}

			kstrncpy(args[num_args], argptr, cmdline - argptr);
			args[num_args++][cmdline - argptr] = '\0';
		}
	}

	switch(pcommand->handler(num_args, args))
	{
		case MON_E_SYNTAX:
			kputs("Syntax error");
			break;

		case MON_E_INVALID_ARG:
			kputs("Invalid argument");
			break;

		case MON_E_NOT_IMPLEMENTED:
			kputs("Not implemented");
			break;

		case MON_E_INTERNAL_ERROR:
			kputs("Internal error");
			break;

		case MON_E_OUT_OF_MEMORY:
			kputs("Out of memory");
			break;

		case MON_E_OK:
			break;
	}

	for(c = 0; c < num_args; ++c)
		kfree(args[c]);
}

