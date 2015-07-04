#ifndef _SECTOR_READ_H_
#define _SECTOR_READ_H_
/*
	Test harness for MBR-reading code

	Stuart Wallace, July 2012.
*/

#include "include/defs.h"		/* from OS code */

int read_sector(const unsigned int num, void *buf);

#endif

