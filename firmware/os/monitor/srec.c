/*
	S-Record interpreter

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "srec.h"
#include "kutil/kutil.h"
#include "memory/kmalloc.h"


static u32 readhex(ks8 * const p, ku32 num)
{
	u32 i, accum;

	for(accum = 0, i = 0; i < num; ++i)
	{
		accum <<= 4;
		if((p[i] >= '0') && (p[i] <= '9'))
			accum += p[i] - '0';
		else if((p[i] >= 'a') && (p[i] <= 'f'))
			accum += 10 + p[i] - 'a';
		else if((p[i] >= 'A') && (p[i] <= 'F'))
			accum += 10 + p[i] - 'A';
	}

	return accum;
}


static s32 srec_err(struct srec_data * const s, const enum srec_error e)
{
	s->error = e;
	if(s->data)
	{
		kfree(s->data);
		s->data = NULL;
	}
	return 1;
}


ks8 * const srec_strerror(const enum srec_error e)
{
	switch(e)
	{
		case se_ok:						return "No error";
		case se_illegal_character:		return "Illegal character";
		case se_bad_line_length:		return "Bad line length";
		case se_bad_checksum:			return "Bad checksum";
		case se_bad_record_type:		return "Bad record type";
		case se_records_out_of_order:	return "Records out of order";
		case se_out_of_memory:			return "Out of memory";
		case se_bad_record_count:		return "Bad record count";
		case se_missing_eob_record:		return "Missing end-of-block record";
		case se_bad_start_address:		return "Bad start address";
		default:						return "Unknown error";
	}
}


s32 srec(struct srec_data *s)
{
	s8 line[SREC_MAX_LINE_LENGTH];
	u32 buf_len = 0;

	s->start_address = 0;
	s->offset = 0;
	s->data_records = 0;
	s->data = NULL;
	s->data_len = 0;
	s->line = 0;
	s->error = se_ok;

	for(;;)
	{
		size_t len;
		u8 record_type;
		u32 i, sum, byte_count, checksum;

		readline(line, SREC_MAX_LINE_LENGTH - 1, 0);
		++(s->line);

		if(!(len = kstrlen(line)))
			continue;

		--len;
		while(len && ((line[len] == 0x0d) || (line[len] == 0x0a)
						|| (line[len] == ' ') || (line[len] == '\t')))
			line[len--] = '\0';
		++len;

		if(!len)
			continue;	/* line comprised only whitespace */

		if((len < 8) || (len & 1))	/* Syntax error: line too short or has an odd num of chars */
			return srec_err(s, se_bad_line_length);

		if((line[1] < '0') || (line[1] > '9') || (line[1] == '6'))
			return srec_err(s, se_bad_record_type);

		if(line[0] != 'S')
			return srec_err(s, se_illegal_character);

		for(i = 2; i < len; ++i)
			if(!isxdigit(line[i]))
				return srec_err(s, se_illegal_character);	/* Syntax error - non-hex digit */

		record_type = line[1] - '0';
		byte_count = readhex(line + 2, 2);
		checksum = readhex(line + len - 2, 2);

		/* Check that line length is correct */
		if(byte_count != ((len - 4) >> 1))
			return srec_err(s, se_bad_line_length);	/* Syntax error - line length incorrect */

		/* Check checksum */
		for(sum = 0, i = 2; i < (len - 2); i += 2)
			sum += readhex(line + i, 2);
		sum = ~sum & 0xff;

		if(sum != checksum)
			return srec_err(s, se_bad_checksum);	/* Bad checksum */

		if(!record_type)									/* Block header - ignore */
			continue;
		else if((record_type >= 1) && (record_type <= 3))	/* Data sequence */
		{
			u32 address = readhex(line + 4, 2 + (record_type << 1));
			byte_count = byte_count - (1 + (record_type + 1));
			s8 *line_ = line + 4 + 2 + (record_type << 1);

			/* The offset calculation assumes that the first record has the lowest address */
			if(!s->data_records)
				s->offset = address;
			else if(address < s->offset)
				return srec_err(s, se_records_out_of_order);	/* Records out of order */

			address -= s->offset;

			if((address + byte_count) > buf_len)
			{
				/* extend buffer */
				ku32 i = 16384;
				if((s->data = krealloc(s->data, buf_len + i)) == NULL)
					return srec_err(s, se_out_of_memory);	/* Out of memory */

				buf_len += i;
			}

			if((address + byte_count) > s->data_len)
				s->data_len = address + byte_count;

			for(i = byte_count; i--; line_ += 2)
				s->data[address++] = readhex(line_, 2);

			++s->data_records;
		}
		else if(record_type == 5)							/* Record count */
		{
			u32 address = readhex(line + 4, 4);
			if(address != s->data_records)
				return srec_err(s, se_bad_record_count);	/* Incorrect record count */
		}
		else if(record_type >= 7)							/* End of block */
		{
			s->start_address = readhex(line + 4, (11 - record_type) << 1) - s->offset;
			if(s->start_address >= s->data_len)
				return srec_err(s, se_bad_start_address);

			kbzero(s->data + s->data_len, buf_len - s->data_len);

			return 0;
		}
	}

	return srec_err(s, se_missing_eob_record);	/* Missing end-of-block record */
}

