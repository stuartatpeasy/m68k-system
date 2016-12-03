#ifndef PLATFORM_PLATFORM_H_INC
#define PLATFORM_PLATFORM_H_INC
/*
	Platform call specification

	A compliant hardware platform must implement all of these.


	Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#define IN_PLATFORM_H

#include <kernel/include/cpu.h>
#include <kernel/include/defs.h>
#include <kernel/include/memory/extents.h>
#include <kernel/include/types.h>
#include <kernel/device/device.h>

/* Parse platform-specific header */
#include <platform/platform_specific.h>


#define LED_RED             (0x80)
#define LED_GREEN           (0x40)
#define LED_ALL             (0xff)

extern mem_extent_t *g_mem_extents;
extern mem_extent_t *g_mem_extents_end;

/*
    NOTE: plat_init() and plat_mem_detect() are called before the initialisation of the kernel heap
    and slabs.  They must not allocate any memory on the heap.
*/
s32 plat_init(void);		        /* Perform any post-reset platform init tasks   	        */
s32 plat_mem_detect();			    /* Populate g_mem_extents with type & location of memory    */

s32 plat_get_serial_number(u8 sn[8]);   /* Place an eight-byte hardware serial number in sn		*/

s32 plat_install_timer_irq_handler(irq_handler handler);

/*
    Quantum (=time-slice) start/stop functions.  Platform code can define these as macros for better
    performance; if so, the platform-specific header should #define PLATFORM_QUANTUM_USES_MACROS.
    If not, the platform-specific code should implement the following two functions.
*/
#ifndef PLATFORM_QUANTUM_USES_MACROS
void plat_stop_quantum();     /* Stop the currently-running quantum (task time-slice)     */
void plat_start_quantum();    /* Start a new quantum                                      */
#endif

s32 plat_dev_enumerate();

s32 plat_get_cpu_clock(u32 *clk);   /* Get CPU clock frequency in Hz                            */

/*
	Base

	The platform must provide one red LED and one green LED, to be used as status indicators.  It
	must provide a means of estimating the system clock frequency, and a means of resetting the CPU.
	It must provide a function which returns a string containing the name of the platform.
*/
s32 plat_led_on(ku8 leds);
s32 plat_led_off(ku8 leds);
s32 plat_clockfreq_detect();
const char * plat_get_name();
void plat_reset() __attribute__((noreturn));


/*
	Console

	The platform must provide a character-oriented console.  This console must be capable of
	accepting and displaying characters generated by the kernel (e.g. messages indicating the
	progress of the boot process, and system messages); it must also be able to capture and return
	user input.

	The console must be available very soon after the start of the boot process.  These functions
	will be called as soon as memory is initialised.

*/

s32 plat_console_init();			    /* Initialise console									*/

#undef IN_PLATFORM_H

#endif