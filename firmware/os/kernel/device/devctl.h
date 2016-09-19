#ifndef KERNEL_DEVICE_DEVCTL_H_INC
#define KERNEL_DEVICE_DEVCTL_H_INC
/*
	devctl.h: device-manipulation functions, constants, etc.

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

/*
	Generic devctl codes.  All codes below 0x00010000 are considered generic.
	They may be implemented by any driver.
*/
typedef enum devctl_fn
{
    /* Block devices */
    dc_get_extent               = 0x0001,	/* # addressable blocks in device: *out = = u32, n	*/
    dc_get_block_size	        = 0x0002,	/* Bytes per block: *out = = u32, n					*/
    dc_get_bootable		        = 0x0003,	/* Is device bootable?	*out = = u32, 1:0			*/
    dc_get_model                = 0x0004,   /* Get device model name                            */
    dc_get_serial               = 0x0005,   /* Get device serial number                         */
    dc_get_firmware_ver         = 0x0006,   /* Get device firmware version                      */
    dc_get_partition_type       = 0x0007,   /* Get partition type                               */
    dc_get_partition_type_name  = 0x0008,   /* Get partition type name                          */
    dc_get_partition_active     = 0x0009,   /* Get partition active flag                        */

    /* Serial ports */
    dc_get_baud_rate            = 0x0100,   /* Get device baud rate                             */
    dc_set_baud_rate            = 0x0101,   /* Set device baud rate                             */

    /* Network devices */
    dc_get_hw_protocol          = 0x0200,   /* Get hardware protocol                            */
    dc_get_hw_addr_type         = 0x0201,   /* Get hardware address type                        */
    dc_get_hw_addr              = 0x0202,   /* Get hardware address                             */
    dc_get_proto_addr_type      = 0x0203,   /* Get protocol address type                        */
    dc_get_proto_addr           = 0x0204,   /* Get protocol address                             */
    dc_get_link_flags           = 0x0205,   /* Get link status flags                            */

    /* Keyboards */
    dc_get_leds                 = 0x0300,   /* Get keyboard LED status                          */
    dc_set_leds                 = 0x0301,   /* Set keyboard LED status                          */

    /* Generic devctls */
    dc_get_power_state          = 0x8000,   /* Get power state for a device                     */
    dc_set_power_state          = 0x8001    /* Set power state for a device                     */
} devctl_fn_t;

#endif
