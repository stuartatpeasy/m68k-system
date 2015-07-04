#include <stdio.h>
#include <stdlib.h>
#include "srec.h"

FILE *fp = NULL;


char *readline()
{
	static char line[4096];
	if(!fgets(line, sizeof(line), fp))
		return NULL;

	return line;
}


int main(int argc, char **argv)
{
	struct srec_data s;

	if(argc != 2)
	{
		fprintf(stderr, "Syntax: %s <file>\n", argv[0]);
		return 1;
	}

	if((fp = fopen(argv[1], "r")) == NULL)
	{
		fprintf(stderr, "Failed to open %s for reading\n", argv[1]);
		return 2;
	}

	if(!srec(&s))
	{
		int i = 0;

		printf("s->data=%p, data_len=%zu\n", s.data, s.data_len);

		while(i < s.data_len)
		{
			int j;
			printf("%06x:  ", i);
			for(j = 0; j < 32; j += 2)
				printf("%02x%02x ", s.data[i + j], s.data[i + j + 1]);
			putchar('\n');
			i += 32;
		}

		printf("Start address = %x\nOffset = %x\nData records = %u\nData = %p\nData len = %zu\nLine = %u\nError = %s\n", s.start_address, s.offset, s.data_records, s.data, s.data_len, s.line, srec_strerror(s.error));

	}
	else
		printf("srec() error: %s\n", srec_strerror(s.error));

	fclose(fp);

	return 0;
}

