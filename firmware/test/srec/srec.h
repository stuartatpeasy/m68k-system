#ifndef __SREC_H__
#define __SREC_H__
/*

*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


enum srec_error
{
	se_ok = 0,
	se_illegal_character,
	se_bad_line_length,
	se_bad_checksum,
	se_records_out_of_order,
	se_out_of_memory,
	se_bad_record_count,
	se_missing_eob_record,
	se_bad_start_address
};


struct srec_data
{
	unsigned int	start_address;
	unsigned int	offset;
	unsigned int	data_records;
	unsigned char	*data;
	size_t			data_len;
	unsigned int	line;
	enum srec_error	error;
};


const char * const srec_strerror(const enum srec_error e);
int srec(struct srec_data *s);

#endif

