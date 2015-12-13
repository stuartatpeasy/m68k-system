#ifndef PORTDEFS_H_
#define PORTDEFS_H_
/*
	portdefs.h - I/O port definitions for the PS/2 keyboard & mouse controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

/*
	Device (ATmega8L) pinout
	------------------------

PD7		AIN1			13		i/o		MOUSE_DATA
PD6		AIN0			12		i/o		KB_DATA
PD5		T1				11		i/o		nIRQ
PD4		XCK/T0			 6		i		nCS
PD3		INT1			 5		i		MOUSE_CLK
PD2		INT0			 4		i		KB_CLK
PD1		TXD				 3		o		nACK			[debug: TXD]
PD0		RXD				 2		i		nID				[debug: RXD]

PC6		nRESET			 1		i		nRESET
PC5		ADC5/SCL		28		i		nUW
PC4		ADC4/SDA		27		i		nUR
PC3		ADC3			26		i		A4
PC2		ADC2			25		i		A3
PC1		ADC1			24		i		A2
PC0		ADC0			23		i		A1

PB7		XTAL2/TOSC2		10		i/o		D15
PB6		XTAL1/TOSC1		 9		i/o		D14
PB5		SCK				19		i/o		D13
PB4		MISO			18		i/o		D12
PB3		MOSI/OC2		17		i/o		D11
PB2		nSS/OC1B		16		i/o		D10
PB1		OC1A			15		i/o		D9
PB0		ICP1			14		i/o		D8
*/


/* Port definitions */
#define DATA_BUS			PORTB
#define DATA_BUS_DDR		DDRB
#define DATA_BUS_PIN		PINB

#define KB_PORT				PORTD
#define KB_DDR				DDRD
#define MOUSE_PORT			PORTD
#define MOUSE_DDR			DDRD

/* Port D (ADDR_PS2) pins */
#define MOUSE_DATA			_BV(7)		/* PS/2 mouse data					*/
#define KB_DATA				_BV(6)		/* PS/2 keyboard data				*/
#define nIRQ				_BV(5)		/* IRQ request output - open-drain	*/
#define nCS					_BV(4)		/* Chip select input from host		*/
#define MOUSE_CLK			_BV(3)		/* PS/2 mouse clock					*/
#define KB_CLK				_BV(2)		/* PS/2 keyboard clock				*/
#define nACK				_BV(1)		/* IRQ acknowledge input from host	*/
#define nID					_BV(0)		/* ID request input from host		*/

#define PORTD_OUTPUTS		(nACK)
#define PORTD_PULLUPS		(KB_CLK | MOUSE_CLK | KB_DATA | MOUSE_DATA)

/* Port C (BUS_CTL) pins */
#define nRESET				_BV(6)		/* nRESET input from host system	*/
#define nW					_BV(5)		/* Upper byte write input from host	*/
/*							_BV(4)		   --- unused ---					*/
#define A4					_BV(3)		/* Host address A4					*/
#define A3					_BV(2)		/* Host address A3					*/
#define A2					_BV(1)		/* Host address A2					*/
#define A1					_BV(0)		/* Host address A1					*/

#define PORTC_ADDR_MASK		(A4 | A3 | A2 | A1)
#define PORTC_ADDR_SHIFT	(0)

/*
	Note: nIRQ is treated as an input.  nIRQ is an open-drain output, and therefore only becomes an
	output when it is asserted.  At all other times, it is an input.
*/
#define PORTC_OUTPUTS		(0)			/* All port C pins are inputs		*/
#define PORTC_PULLUPS		(0)			/* All port C pins are driven		*/

/* Port B (DATA_BUS) pins */
#define DATA_BUS_OUTPUTS	(0)			/* All pins are initially inputs	*/
#define DATA_BUS_PULLUPS	(0)

#endif /* PORTDEFS_H_ */