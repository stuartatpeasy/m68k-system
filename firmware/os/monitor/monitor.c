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
	{"go",				cmd_go},
	{"help",			cmd_help},
	{"history",         cmd_history},
	{"id",              cmd_id},
	{"ls",              cmd_ls},
	{"map",             cmd_map},
	{"mount",           cmd_mount},
	{"raw",				cmd_raw},
	{"rootfs",          cmd_rootfs},
	{"schedule",        cmd_schedule},
	{"serial",          cmd_serial},
	{"slabs",           cmd_slabs},
	{"srec",			cmd_srec},
	{"test",            cmd_test},
	{"upload",			cmd_upload},
	{"write",			cmd_write},
	{"writeh",			cmd_writeh},
	{"writew",			cmd_writew},

	{NULL, NULL}
};


void monitor(void)
{
    history_init();

    for(;;)
        monitor_main();
}


void monitor_main(void)
{
	char buffer[CMD_MAX_LEN + 1];

    g_echo = 1;
	putchar('\n');

	for(buffer[CMD_MAX_LEN] = '\0'; ;)
	{
		put(g_prompt);
		readline(buffer, CMD_MAX_LEN, g_echo);

		if(strlen(buffer))
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

	cmd_len = strlen(command);
	if(!cmd_len)
		return;

    history_add(cmdline);

	const struct command *p, *pcommand = NULL;
	for(p = g_commands; p->name; ++p)
    {
		if(strstr(p->name, command) == p->name)
		{
			if(pcommand)
			{
				puts("Ambiguous command");
				return;
			}
			else
			{
				pcommand = p;
				if(cmd_len == strlen(p->name))
					break;	/* exact match */
			}
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
				puts("Too many arguments");
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
				puts("Out of memory");
				return;
			}

			strncpy(args[num_args], argptr, cmdline - argptr);
			args[num_args++][cmdline - argptr] = '\0';
		}
	}

	switch(pcommand->handler(num_args, args))
	{
		case MON_E_SYNTAX:
			puts("Syntax error");
			break;

		case MON_E_INVALID_ARG:
			puts("Invalid argument");
			break;

		case MON_E_NOT_IMPLEMENTED:
			puts("Not implemented");
			break;

		case MON_E_INTERNAL_ERROR:
			puts("Internal error");
			break;

		case MON_E_OUT_OF_MEMORY:
			puts("Out of memory");
			break;

		case MON_E_OK:
			break;
	}

	for(c = 0; c < num_args; ++c)
		kfree(args[c]);
}

