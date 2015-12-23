#ifndef PORTDEFS_H_
#define PORTDEFS_H_
/*
	portdefs.h - I/O port definitions for the dual-channel PS/2 controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

/*
	Device (ATmega8L) pinout
	------------------------

PD7		AIN1			13		i/o		CHAN_B_DATA
PD6		AIN0			12		i/o		CHAN_A_DATA
PD5		T1				11		i/o		nIRQ
PD4		XCK/T0			 6		i		nCS
PD3		INT1			 5		i		CHAN_B_CLK
PD2		INT0			 4		i		CHAN_A_CLK
PD1		TXD				 3		o		nACK			[debug: TXD]
PD0		RXD				 2		i		nID				[debug: RXD]

PC6		nRESET			 1		i		nRESET
PC5		ADC5/SCL		28		i		nW
PC4		ADC4/SDA		27		i		nPWR_B
PC3		ADC3			26		i		nPWR_A
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
#define DATA_BUS_PORT		PORTB
#define DATA_BUS_DDR		DDRB
#define DATA_BUS_PIN		PINB

#define CHAN_A_PORT			PORTD
#define CHAN_A_PIN			PIND
#define CHAN_A_DDR			DDRD
#define CHAN_B_PORT			PORTD
#define CHAN_B_PIN			PIND
#define CHAN_B_DDR			DDRD
#define IRQ_PORT			PORTD
#define IRQ_DDR				DDRD
#define PWR_PORT			PORTC

/* Port D pins */
#define CHAN_B_DATA			_BV(7)		/* PS/2 channel B data				*/
#define CHAN_A_DATA			_BV(6)		/* PS/2 channel A data				*/
#define nIRQ				_BV(5)		/* IRQ output - open-drain			*/
#define nCS					_BV(4)		/* Chip select input from host		*/
#define CHAN_B_CLK			_BV(3)		/* PS/2 channel B clock				*/
#define CHAN_A_CLK			_BV(2)		/* PS/2 channel A clock				*/
#define nACK				_BV(1)		/* Bus transfer acknowledge output	*/
#define nID					_BV(0)		/* ID request input from host		*/

#define PORTD_OUTPUTS		(nACK)
#define PORTD_PULLUPS		(CHAN_A_CLK | CHAN_B_CLK | CHAN_A_DATA | CHAN_B_DATA)	// FIXME - remove

/* Port C pins */
#define nRESET				_BV(6)		/* nRESET input from host system	*/
#define nW					_BV(5)		/* Upper byte write input from host	*/
#define nPWR_A				_BV(4)		/* Channel A power enable			*/
#define nPWR_B				_BV(3)		/* Channel B power enable			*/
#define A3					_BV(2)		/* Host address A3					*/
#define A2					_BV(1)		/* Host address A2					*/
#define A1					_BV(0)		/* Host address A1					*/

#define PORTC_ADDR_MASK		(A3 | A2 | A1)
#define PORTC_ADDR_SHIFT	(0)

/*
	Note: nIRQ is treated as an input.  nIRQ is an open-drain output, and therefore only becomes an
	output when it is asserted.  At all other times, it is an input.
*/
#define PORTC_OUTPUTS		(nPWR_A | nPWR_B)	/* Power switches are outputs		*/
#define PORTC_PULLUPS		(0)					/* All port C pins are driven		*/

/* Port B (DATA_BUS) pins */
#define DATA_BUS_OUTPUTS	(0)					/* All pins are initially inputs	*/
#define DATA_BUS_PULLUPS	(0)

#endif /* PORTDEFS_H_ */