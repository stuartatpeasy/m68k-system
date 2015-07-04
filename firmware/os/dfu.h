#ifndef __DFU_H__
#define __DFU_H__
/*
	Device firmware update functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, January 2012.
*/

#include "include/types.h"
#include "memory/kmalloc.h"
#include "kutil/kutil.h"
#include "asm/reset.h"
#include "duart.h"			/* for led_on() */


#define FLASH_BASE			(0xf00000)

#define FLASH_OFFSET(x)		*((vu16 *) (FLASH_BASE + (x << 1)))
#define FLASH_REG1			*((vu16 *) (FLASH_BASE + (0x555 << 1)))
#define FLASH_REG2			*((vu16 *) (FLASH_BASE + (0x2aa << 1)))


#define DFU_OK				(0)
#define DFU_NO_DATA			(1)
#define DFU_BAD_DATA_LEN	(2)
#define DFU_OUT_OF_MEMORY	(3)
#define DFU_UNKNOWN_ERROR	(4)


int dfu(ku16 *data, ku32 len);

#endif

