/*
	slowcat: a slow version of 'cat'

	(c) Stuart Wallace, February 2012


	This tool writes data to stdout one character at a time, slowly.  It works around a problem
	where some USB-based RS-232 interfaces do not properly respect RTS/CTS handshaking by sending
	characters slowly enough that handshaking is not required.
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h>


double tod(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec + ((double) tv.tv_usec / 1000000.0);
}


int main(int argc, char **argv)
{
	int opt;
	char *endptr;
	double delay = 0.001;		/* Default inter-char delay is 1ms */

	while((opt = getopt(argc, argv, "c:d:")) != -1)
	{
		switch(opt)
		{
			case 'c':
				errno = 0;
				delay = strtod(optarg, &endptr);
				
				if(((errno == ERANGE) && (delay == HUGE_VALL || delay == -HUGE_VALL))
					|| (errno && !delay) || !delay)
				{
					fputs("Invalid character rate", stderr);
					return 1;
				}

				delay = 1.0 / delay;
				break;

			case 'd':
				errno = 0;
				delay = strtod(optarg, &endptr);
				
				if(((errno == ERANGE) && (delay == HUGE_VALL || delay == -HUGE_VALL))
					|| (errno && !delay))
				{
					fputs("Invalid delay value", stderr);
					return 1;
				}
				break;

			default:
				fprintf(stderr, "Syntax: %s [-d <delay>] [files...]\n"
								"        %s [-c <chars/sec> [files...]\n", argv[0], argv[0]);
				return 1;
				break;
		}
	}

	do
	{
		int c;
		double t;
		FILE *fp = (optind < argc) ? fopen(argv[optind], "r") : stdin;
		
		if(fp == NULL)
		{
			fprintf(stderr, "Cannot open '%s': %s\n", argv[optind], strerror(errno));
			return 2;
		}

		while((c = fgetc(fp)) != EOF)
		{
			t = tod();

			fputc(c, stdout);
			fflush(stdout);

			while(tod() < (t + delay))
				;
		}

		fclose(fp);
	}
	while(++optind < argc);

	return 0;
}

