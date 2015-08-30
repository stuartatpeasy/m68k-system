/*
	Platform call specification

	A compliant hardware platform must implement all of these.


	Stuart Wallace <stuartw@atom.net>, August 2015.

	NOTE: this is currently just a collection of thoughts.  It is not yet ready for use!
*/

/*
	Base

	The platform must be able to detect the amount of installed RAM.  It must provide one red LED
	and one green LED, to be used as status indicators.  It must be provide a means of estimating
	the system clock frequency.
*/
s32 base_ram_detect();
s32 base_led_on(u32 leds);
s32 base_led_off(u32 leds);
s32 base_clockfreq_detect();
s32 base_hwinfo(hwinfo_t *);


/*
	Console

	The platform must provide a character-oriented console.  This console must be capable of
	accepting and displaying characters generated by the kernel (e.g. messages indicating the
	progress of the boot process, and system messages); it must also be able to capture and return
	user input.
*/

s32 console_init();					/* Initialise console										*/
s32 console_putchar(s32 c);			/* Write a character to the console.  May block.			*/
s32 console_getchar(void);			/* Read a character from the console.  Blocking.			*/
s32 console_hwinfo(hwinfo_t *);


/*
	NVRAM

	The platform must support at least 64 bytes of non-volatile (i.e. battery-backed) RAM, in
	which configuration data may be stored.

*/

s32 nvram_init();
s32 nvram_read(u32 addr, u32 len, void *buffer);		/* Read len bytes from offset addr 		*/
u32 nvram_write(u32 addr, u32 len, const void *buffer);	/* Write len bytes at offset addr		*/
s32 nvram_hwinfo(hwinfo_t *);


/*
	RTC

	The platform must expose a real-time clock with resolution of at least one second.  This clock
	must be readable and settable via a struct rtc_time.
*/

s32 rtc_init();
s32 rtc_get_time(rtc_time_t * const tm);
s32 rtc_set_time(const rtc_time_t * const tm);
s32 rtc_hwinfo(hwinfo_t *);


/*
	ATA

	The platform must expose at least one dual-channel ATA interface.

	TODO: work out what to do about the anonymous "data" var in read/write fns.
*/

s32 ata_init();
s32 ata_read(void *data, u32 first_sector, u32 nsectors, void *buffer);
s32 ata_write(void *data, u32 first_sector, u32 nsectors, const void *buffer);
s32 ata_hwinfo(hwinfo_t *);
