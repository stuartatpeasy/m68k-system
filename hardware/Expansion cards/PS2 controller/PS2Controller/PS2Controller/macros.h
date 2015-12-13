#ifndef MACROS_H_
#define MACROS_H_
/*
	macros.h - Macro definitions for the PS/2 keyboard & mouse controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

#include "portdefs.h"


/* Make the data bus pins outputs */
#define DATA_BUS_SET_OUTPUT()		\
	DATA_BUS_DDR = 0xff

/* Make the data bus pins inputs (the default state) */
#define DATA_BUS_SET_INPUT()		\
	DATA_BUS_DDR = 0x00

/* Terminate a m68k bus cycle by asserting nACK and waiting for nCS to negate */
#define TERMINATE_BUS_CYCLE()		\
	do								\
	{								\
		PORTD &= ~nACK;				\
		while(PORTD & nCS)			\
			;						\
									\
		PORTD |= nACK;				\
	} while(0);

/* Respond to a read cycle: write "data" to the data bus outputs, and terminate the cycle */
#define DO_READ_CYCLE(data)			\
	do								\
	{								\
		DATA_BUS_PIN = (data);		\
		DATA_BUS_SET_OUTPUT();		\
		TERMINATE_BUS_CYCLE();		\
		DATA_BUS_SET_INPUT();		\
	} while(0);

#endif /* MACROS_H_ */