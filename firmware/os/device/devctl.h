#ifndef __OS_DEVICE_DEVCTL_H__
#define __OS_DEVICE_DEVCTL_H__
/*
	devctl.h: device-manipulation functions, constants, etc.

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

/*
	Generic devctl codes.  All codes below 0x00010000 are considered generic.
	They may be implemented by any driver.
*/
#define DEVCTL_EXTENT		(0x0001)	/* # blocks in block device: *out = (u32) n				*/
#define DEVCTL_BLOCK_SIZE	(0x0002)	/* Bytes per sector: *out = (u32) n						*/
#define DEVCTL_BOOTABLE		(0x0003)	/* Is device bootable?	*out = (u32) 1:0				*/
#define DEVCTL_MODEL        (0x0004)    /* Get device model name                                */
#define DEVCTL_SERIAL       (0x0005)    /* Get device serial number                             */
#define DEVCTL_FIRMWARE_VER (0x0006)    /* Get device firmware version                          */

#endif

