#ifndef REGISTERS_H_
#define REGISTERS_H_
/*
	registers.h - Register definitions for the PS/2 keyboard & mouse controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */


/*
	Register definitions
*/
#define REG_KB_DATA				(0)		/* Keyboard received data register		*/
#define REG_MOUSE_DATA			(1)		/* Mouse received data register			*/
#define REG_KB_CMD				(2)		/* Keyboard command holding register	*/
#define REG_MOUSE_CMD			(3)		/* Mouse command holding register		*/
#define REG_STATUS				(4)		/* Status flags							*/
#define REG_CFG					(5)		/* Controller configuration				*/
#define REG_INT_CFG				(6)		/* Interrupt-enable flags				*/
#define REG_UNUSED				(7)

/* Flag bits - used in REG_STATUS and REG_INT_CFG */
#define FLAG_KBRX				_BV(7)	/* Byte received from keyboard port		*/
#define FLAG_KBTXDONE			_BV(6)	/* Keyboard command transmit finished	*/
#define FLAG_KBPARERR			_BV(5)	/* Keyboard port receiver parity error	*/
#define FLAG_KBOVF				_BV(4)	/* Keyboard FIFO overflow				*/
#define FLAG_MOUSERX			_BV(3)	/* Byte received from mouse port		*/
#define FLAG_MOUSETXDONE		_BV(2)	/* Mouse command transmit finished		*/
#define FLAG_MOUSEPARERR		_BV(1)	/* Mouse port receiver parity error		*/
#define FLAG_MOUSEOVF			_BV(0)	/* Mouse FIFO overflow					*/

/* Bits in REG_CFG */
#define CFG_IE					_BV(7)	/* Global interrupt enable				*/
#define CFG_PWR_KB				_BV(4)	/* Enable/disable power to keyboard		*/
#define CFG_PWR_MOUSE			_BV(3)	/* Enable/disable power to mouse		*/

#endif