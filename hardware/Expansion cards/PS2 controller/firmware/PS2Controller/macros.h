#ifndef MACROS_H_
#define MACROS_H_
/*
	macros.h - Macro definitions for the dual-channel PS/2 controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

#include "portdefs.h"
#include "types.h"

/* Set the specified pins low or high in the specified port */
#define SET_LOW(port, pins)			((port) &= (u8) ~(pins))
#define SET_HIGH(port, pins)		((port) |= (u8) (pins))

/* Set the pin in the specified port as an output or an input */
#define SET_INPUT(ddr, pins)		SET_LOW((ddr), (pins))
#define SET_OUTPUT(ddr, pins)		SET_HIGH((ddr), (pins))

/* Read the value of the specified pin in the specified port */
#define READ_PIN(port, pin)			((port) & (pin))

/* Assert nIRQ (open-drain output) by making the pin (which is always set low) an output */
#define HOST_IRQ_ASSERT()		SET_OUTPUT(IRQ_DDR, nIRQ);

/* Assert nIRQ if the supplied condition is true */
#define HOST_IRQ_ASSERT_IF(cond)					\
	do												\
	{												\
		if((cond) && (host_regs[REG_CFG] & CFG_IE))	\
			HOST_IRQ_ASSERT();						\
	} while (0);

/* Release nIRQ (open-drain output) by making the pin an input (high-Z) */
#define HOST_IRQ_RELEASE()		SET_INPUT(IRQ_DDR, nIRQ);

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
		while(!(PIND & nCS))		\
			;						\
									\
		SET_HIGH(PORTD, nACK);		\
	} while(0);

/* Respond to a read cycle: write "data" to the data bus outputs, and terminate the cycle */
#define DO_READ_CYCLE(data)			\
	do								\
	{								\
		DATA_BUS_PORT = (data);		\
		DATA_BUS_SET_OUTPUT();		\
		TERMINATE_BUS_CYCLE();		\
		DATA_BUS_SET_INPUT();		\
	} while(0);

/* Read the current address from the address bus inputs */
#define GET_ADDRESS()			((PINC & PORTC_ADDR_MASK) >> PORTC_ADDR_SHIFT)

#endif