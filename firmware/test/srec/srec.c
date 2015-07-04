#include "srec.h"

/* FIXME: remove this */
char *readline();

unsigned int readhex(const char * const p, const unsigned int num)
{
	unsigned int i, accum;

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


const char * const srec_strerror(const enum srec_error e)
{
	switch(e)
	{
		case se_ok:						return "No error";
		case se_illegal_character:		return "Illegal character";
		case se_bad_line_length:		return "Bad line length";
		case se_bad_checksum:			return "Bad checksum";
		case se_records_out_of_order:	return "Records out of order";
		case se_out_of_memory:			return "Out of memory";
		case se_bad_record_count:		return "Bad record count";
		case se_missing_eob_record:		return "Missing end-of-block record";
		case se_bad_start_address:		return "Bad start address";
		default:						return "Unknown error";
	}
}


int srec(struct srec_data *s)
{
	char *line;
	unsigned int buf_len = 0;

	s->start_address = 0;
	s->offset = 0;
	s->data_records = 0;
	s->data = NULL;
	s->data_len = 0;
	s->line = 0;
	s->error = se_ok;

	while((line = readline()))
	{
		size_t len;
		unsigned char record_type;
		unsigned int i, sum, byte_count, checksum;

		++s->line;

		if(!(len = strlen(line)))
			continue;

		--len;
		while(len && ((line[len] == 0x0d) || (line[len] == 0x0a)
						|| (line[len] == ' ') || (line[len] == '\t')))
			line[len--] = '\0';
		++len;

		if(!len)
			continue;	/* line comprised only whitespace */

		if((len < 8) || 	/* Syntax error - line too short, or...							*/
		   (len & 1) ||								/* ...has an odd num of characters, or	*/ 
		   (line[0] != 'S') ||						/* ...does not start with an 'S'		*/
		   ((line[1] < '0') || (line[1] > '9')) ||	/* ... \								*/
		    (line[1] == '6'))						/* ... / the record type is invalid		*/
			return 1;

		for(i = 2; i < len; ++i)
			if(!isxdigit(line[i]))
			{
				s->error = se_illegal_character;	/* Syntax error - non-hex digit in line */
				if(s->data)
				{
					free(s->data);
					s->data = NULL;
				}
				return 1;
			}

		record_type = line[1] - '0';
		byte_count = readhex(line + 2, 2);
		checksum = readhex(line + len - 2, 2);

		/* Check that line length is correct */
		if(byte_count != ((len - 4) >> 1))
		{
			s->error = se_bad_line_length;			/* Syntax error - line length incorrect */
			if(s->data)
			{
				free(s->data);
				s->data = NULL;
			}
			return 1;
		}

		/* Check checksum */
		for(sum = 0, i = 2; i < (len - 2); i += 2)
			sum += readhex(line + i, 2);
		sum = ~sum & 0xff;

		if(sum != checksum)
		{
			s->error = se_bad_checksum;				/* Bad checksum */
			if(s->data)
			{
				free(s->data);
				s->data = NULL;
			}
			return 1;
		}

		if(!record_type)									/* Block header - ignore */
			continue;
		else if((record_type >= 1) && (record_type <= 3))	/* Data sequence */
		{
			unsigned int address = readhex(line + 4, 2 + (record_type << 1));
			byte_count = byte_count - (1 + (record_type + 1));
			line += 4 + 2 + (record_type << 1);

			/* The offset calculation assumes that the first record has the lowest address */
			if(!s->data_records)
				s->offset = address;
			else if(address < s->offset)
			{
				s->error = se_records_out_of_order;		/* Records out of order */
				if(s->data)
				{
					free(s->data);
					s->data = NULL;
				}
				return 1;
			}
			address -= s->offset;

			if((address + byte_count) > s->data_len)
			{
				/* extend buffer */
				const unsigned int i = 4096;
				if((s->data = realloc(s->data, buf_len + i)) == NULL)
				{
					s->error = se_out_of_memory;		/* Out of memory */
					if(s->data)
					{
						free(s->data);
						s->data = NULL;
					}
					return 1;
				}
				buf_len += i;

			}

			if((address + byte_count) > s->data_len)
				s->data_len = address + byte_count;

			for(i = byte_count; i--; line += 2)
				s->data[address++] = readhex(line, 2);

			++s->data_records;
		}
		else if(record_type == 5)							/* Record count */
		{
			unsigned int address = readhex(line + 4, 4);
			if(address != s->data_records)
			{
				s->error = se_bad_record_count;		/* Incorrect record count */
				if(s->data)
				{
					free(s->data);
					s->data = NULL;
				}
				return 1;
			}
		}
		else if(record_type >= 7)							/* End of block */
		{
			s->start_address = readhex(line + 4, (11 - record_type) << 1) - s->offset;
			if(s->start_address >= s->data_len)
			{
				s->error = se_bad_start_address;
				if(s->data)
				{
					free(s->data);
					s->data = NULL;
				}
				return 1;
			}
			memset(s->data + s->data_len, 0, buf_len - s->data_len);

			return 0;
		}
	}

	s->error = se_missing_eob_record;
	if(s->data)
	{
		free(s->data);
		s->data = NULL;
	}
	return 1;	/* Missing end-of-block record */
}

