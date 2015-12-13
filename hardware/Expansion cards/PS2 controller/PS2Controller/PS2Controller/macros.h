#ifndef MACROS_H_
#define MACROS_H_
/*
	macros.h - Macro definitions for the PS/2 keyboard & mouse controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

#include "portdefs.h"

/* Set the specified pins low or high in the specified port */
#define SET_LOW(port, pins)			((port) &= ~(pins))
#define SET_HIGH(port, pins)		((port) |= (pins))

/* Set the edge on which INT0/INT1 interrupts trigger */
#define INT0_SET_FALLING_EDGE()	(MCUCR = (MCUCR | _BV(ISC01)) & ~_BV(ISC00))
#define INT0_SET_RISING_EDGE()	(MCUCR |= _BV(ISC01) | _BV(ISC00))
#define INT1_SET_FALLING_EDGE()	(MCUCR = (MCUCR | _BV(ISC11)) & ~_BV(ISC10))
#define INT1_SET_RISING_EDGE()	(MCUCR |= _BV(ISC11) | _BV(ISC10))

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
		SET_LOW(PORTD, nACK);		\
		while(PORTD & nCS)			\
			;						\
									\
		SET_HIGH(PORTD, nACK);		\
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

/*
	Macros for reading and manipulating various I/O lines
*/

/* Read the current address from the address bus inputs */
#define GET_ADDRESS()			((PORTC & PORTC_ADDR_MASK) >> PORTC_ADDR_SHIFT)

/* Make the keyboard clock/data line an input or an output */
#define KB_CLK_SET_OUTPUT()		SET_HIGH(KB_DDR, KB_CLK)
#define KB_CLK_SET_INPUT()		SET_LOW(KB_DDR, KB_CLK)
#define KB_DATA_SET_OUTPUT()	SET_HIGH(KB_DDR, KB_DATA)
#define KB_DATA_SET_INPUT()		SET_LOW(KB_DDR, KB_DATA)

/* Make the keyboard data line an input or an output */
#define MOUSE_CLK_SET_OUTPUT()	SET_HIGH(MOUSE_DDR, MOUSE_CLK)
#define MOUSE_CLK_SET_INPUT()	SET_LOW(MOUSE_DDR, MOUSE_CLK)
#define MOUSE_DATA_SET_OUTPUT()	SET_HIGH(MOUSE_DDR, MOUSE_DATA)
#define MOUSE_DATA_SET_INPUT()	SET_LOW(MOUSE_DDR, MOUSE_DATA)

/* Set the keyboard clock/data pin values, assuming they have already been set as an output */
#define KB_CLK_SET_LOW()		SET_LOW(KB_PORT, KB_CLK)
#define KB_CLK_SET_HIGH()		SET_HIGH(KB_PORT, KB_CLK)
#define KB_DATA_SET_LOW()		SET_LOW(KB_PORT, KB_DATA)
#define KB_DATA_SET_HIGH()		SET_HIGH(KB_PORT, KB_DATA)

/* Set the mouse clock/data pin values, assuming they have already been set as an output */
#define MOUSE_CLK_SET_LOW()		SET_LOW(MOUSE_PORT, MOUSE_CLK)
#define MOUSE_CLK_SET_HIGH()	SET_HIGH(MOUSE_PORT, MOUSE_CLK)
#define MOUSE_DATA_SET_LOW()	SET_LOW(MOUSE_PORT, MOUSE_DATA)
#define MOUSE_DATA_SET_HIGH()	SET_HIGH(MOUSE_PORT, MOUSE_DATA)

/* Enable/disable keyboard/mouse interrupts */
#define KB_IRQ_ENABLE()			SET_HIGH(GICR, _BV(INT0))
#define KB_IRQ_DISABLE()		SET_LOW(GICR, _BV(INT0))
#define MOUSE_IRQ_ENABLE()		SET_HIGH(GICR, _BV(INT1))
#define MOUSE_IRQ_DISABLE()		SET_LOW(GICR, _BV(INT1))

#endif