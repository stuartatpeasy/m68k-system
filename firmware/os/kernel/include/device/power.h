#ifndef KERNEL_DEVICE_POWER_H_INC
#define KERNEL_DEVICE_POWER_H_INC
/*
	power.h: declarations related to power management.

	Part of ayumos


	(c) Stuart Wallace <stuartw@atom.net>, September 2016.
*/


/*
    Power states: used by peripherals for power management.
*/
typedef enum
{
    PWR_STATE_NORMAL,   /* Highest power state: full functionality, no power-saving */
    PWR_STATE_OFF       /* Minimal power state: no functionality available          */
} PwrState;

#endif
