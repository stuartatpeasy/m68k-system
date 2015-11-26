/*
	Implementation of the readline() function: reads a line from the serial port

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011-2012.


	TODO: get rid of "echo" stuff; implement separate length-limited read function.
*/

#include <kernel/platform.h>
#include <monitor/readline.h>


#define MAX_LINE_LENGTH 	(255)

#define KEY_CTRL_a			(0x01)		/* Jump to start of line					*/
#define KEY_CTRL_b			(0x02)		/* Cursor left (backwards)					*/
#define KEY_CTRL_c			(0x03)		/* Cancel input								*/
#define KEY_CTRL_e			(0x05)		/* Jump to end of line						*/
#define KEY_CTRL_f			(0x06)		/* Cursor right (forwards)					*/
#define KEY_CTRL_g			(0x07)		/* Bell - echo but do not add to line		*/
#define KEY_CTRL_h			(0x08)		/* Backspace								*/
#define KEY_CTRL_j			(0x0a)		/* Line feed								*/
#define KEY_CTRL_k			(0x0b)		/* Kill from cursor pos to end of line		*/
#define KEY_CTRL_m			(0x0d)		/* Carriage return							*/
#define KEY_CTRL_u			(0x15)		/* Kill from start of line to cursor pos	*/
#define KEY_CTRL_w			(0x17)		/* Kill word backwards						*/

#define READLINE_GETC()			plat_console_getc()
#define READLINE_PUTC(x, echo)	{ if(echo) plat_console_putc(x); }


void readline(char *buffer, s32 buffer_len, ku32 echo)
{
	int pos = 0, line_length = 0;

	buffer[0] = '\0';
	while(1)
	{
		const char c = READLINE_GETC();

		switch(c)
		{
			case KEY_CTRL_a:					/* Jump to start of line */
				for(; pos; --pos)
					READLINE_PUTC(0x08, echo);
				break;

			case KEY_CTRL_b:					/* Cursor left (backwards) */
				if(pos)
				{
					READLINE_PUTC(0x08, echo);
					--pos;
				}
				break;

			case KEY_CTRL_c:					/* Cancel line */
				READLINE_PUTC('^', echo);
				READLINE_PUTC('C', echo);
				READLINE_PUTC('\n', echo);
				buffer[0] = '\0';
				return;

			case KEY_CTRL_e:					/* Jump to end of line */
				for(; pos < line_length; ++pos)
					READLINE_PUTC(buffer[pos], echo);
				break;

			case KEY_CTRL_f:
				if(pos < line_length)
					READLINE_PUTC(buffer[pos++], echo);
				break;

			case KEY_CTRL_g:
				READLINE_PUTC(c, echo);
				break;

			case KEY_CTRL_k:					/* Kill from pos to end of line */
				if(pos < line_length)
				{
					int x;

					buffer[pos] = '\0';
					for(x = pos; x < line_length; ++x)
						READLINE_PUTC(0x20, echo);

					for(x = pos; x < line_length; ++x)
						READLINE_PUTC(0x08, echo);

					line_length = pos;
				}
				break;

			case KEY_CTRL_u:					/* Kill from start of line to pos */
				if(pos)
				{
					int x;

					for(x = pos; x--;)
						READLINE_PUTC(0x08, echo);

					for(x = pos; x < line_length; ++x)
					{
						buffer[x - pos] = buffer[x];
						READLINE_PUTC(buffer[x - pos], echo);
					}

					buffer[x] = '\0';

					for(x = pos; x--;)
						READLINE_PUTC(0x20, echo);

					for(x = line_length; x--;)
						READLINE_PUTC(0x08, echo);

					line_length -= pos;
					pos = 0;
				}
				break;

			case KEY_CTRL_j:					/* 0x0a (LF) */
				/* swallow any preceding CR characters */
				for(; line_length && (buffer[line_length] == '\x0d'); --line_length)
					buffer[line_length] = '\0';

				/* fall through */

			case KEY_CTRL_m:					/* 0x0d (CR) */
				READLINE_PUTC('\x0a', echo);
				buffer[line_length] = 0;

				/* TODO write line to history */
				return;

			case 0x1b:							/* Esc ... */
				{
					switch(READLINE_GETC())
					{
						case 0x5b:				/* [ */
							switch(READLINE_GETC())
							{
                                case 0x31:
                                    switch(READLINE_GETC())
                                    {
                                        case 0x7e:
                                            for(; pos; --pos)
                                                READLINE_PUTC(0x08, echo);
                                            break;
                                    }
                                    break;

								case 0x33:
									break;

                                case 0x34:
                                    switch(READLINE_GETC())
                                    {
                                        case 0x7e:
                                            for(; pos < line_length; ++pos)
                                                READLINE_PUTC(buffer[pos], echo);
                                            break;
                                    }
                                    break;


								case 0x41:						/* Cursor up */
									/* TODO Fetch previous entry from history */
									break;

								case 0x42:						/* Cursor down */
									/* TODO Fetch next entry from history */
									break;

								case 0x43:						/* Cursor right */
									if(pos < line_length)
										READLINE_PUTC(buffer[pos++], echo);
									break;

								case 0x44:						/* Cursor left */
									if(pos)
									{
										READLINE_PUTC(0x08, echo);
										--pos;
									}
									break;
							}
							break;
					}
				}
				break;

			case 0x7e:							/* Delete forwards */
				if(line_length && (pos < line_length))
				{
					int x;

					for(x = pos; x < line_length; ++x)
					{
						buffer[x] = buffer[x + 1];
						READLINE_PUTC(buffer[x], echo);
					}

					READLINE_PUTC(0x20, echo);
					READLINE_PUTC(0x08, echo);

					--line_length;

					for(x = line_length; x > pos; --x)
						READLINE_PUTC(0x08, echo);
				}
				break;

			case KEY_CTRL_h:
			case 0x7f:							/* Delete character under cursor */
				if(line_length && pos)
				{
					int x;

					READLINE_PUTC(0x08, echo);
					--pos;

					for(x = pos; x < line_length; ++x)
					{
						buffer[x] = buffer[x + 1];
						READLINE_PUTC(buffer[x], echo);
					}

					READLINE_PUTC(0x20, echo);
					READLINE_PUTC(0x08, echo);

					--line_length;

					for(x = line_length; x > pos; --x)
						READLINE_PUTC(0x08, echo);
				}
				break;

			default:							/* Insert the character into the line */
				if(pos < buffer_len - 1)
				{
					int x;
					for(x = line_length; x >= pos; --x)
						buffer[x + 1] = buffer[x];

					READLINE_PUTC(c, echo);
					buffer[pos++] = c;
					++line_length;

					for(x = pos; x < line_length; ++x)
						READLINE_PUTC(buffer[x], echo);

					for(x = pos; x < line_length; ++x)
						READLINE_PUTC(0x08, echo);
				}
				break;
		}
	}
}

