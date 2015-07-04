#ifndef __READLINE_H__
#define __READLINE_H__

#include <stdlib.h>
#include <string.h>


#define READLINE_E_MALLOC				(-1)


typedef struct readline_ctx_struct
{
	char **history;
	int num_lines;			/* 0 = don't capture any history; -1 = capture unlimited history */

} readline_ctx;

#endif

