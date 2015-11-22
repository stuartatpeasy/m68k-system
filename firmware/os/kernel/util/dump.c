/*
    dump.c: dump memory contents to the console

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015.
*/

#include <kernel/util/kutil.h>
#include <ctype.h>
#include <stdio.h>


s32 dump_hex(const void *p, ku32 word_size, ku32 offset, ku32 num_bytes)
{
	ku8 line_length = 16;
	u8 x, y;
	s8 *line;
	u32 line_offset, data, start = (u32) p;

	if((word_size != 4) && (word_size != 2) && (word_size != 1))
		return EINVAL;

	if(!(line = (s8 *) kmalloc(line_length)))
		return ENOMEM;

	for(line_offset = 0; line_offset < num_bytes; line_offset += line_length)
	{
		printf("%06x: ", start + line_offset - offset);
		for(x = 0; x < line_length; x += word_size)
		{
			if((line_offset + x) < num_bytes)
				switch(word_size)
				{
				case 1:
					data = *((u8 *) (start + line_offset + x));
					printf("%02x ", (u8) data);
					line[x] = data;
					break;

				case 2:
					data = *((u16 *) (start + line_offset + x));
					printf("%04x ", (u16) data);
					((u16 *) line)[x >> 1] = (u16) data;
					break;

				case 4:
					data = *((u32 *) (start + line_offset + x));
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
			if((line_offset + x) < num_bytes)
				putchar(isprint(line[x]) ? line[x] : '.');
			else
				putchar(' ');
		}
		puts("");
	}

	kfree(line);

	return 0;
}
