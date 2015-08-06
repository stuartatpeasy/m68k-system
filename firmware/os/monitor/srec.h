#ifndef MONITOR_SREC_H_INC
#define MONITOR_SREC_H_INC
/*
	S-Record interpreter

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 2011.
*/

#include "include/types.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "monitor/readline.h"

#define SREC_MAX_LINE_LENGTH		(256)

enum srec_error
{
	se_ok = 0,
	se_illegal_character,
	se_bad_line_length,
	se_bad_checksum,
	se_bad_record_type,
	se_records_out_of_order,
	se_out_of_memory,
	se_bad_record_count,
	se_missing_eob_record,
	se_bad_start_address
};


struct srec_data
{
	u32				start_address;
	u32				offset;
	u32				data_records;
	u8				*data;
	u32 			data_len;
	u32				line;
	enum srec_error	error;
};


ks8 * const srec_strerror(const enum srec_error e);
s32 srec(struct srec_data *s);

#endif

