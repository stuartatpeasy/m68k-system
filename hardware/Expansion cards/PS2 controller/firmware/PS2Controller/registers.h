#ifndef REGISTERS_H_
#define REGISTERS_H_
/*
	registers.h - Register definitions for the dual-channel PS/2 controller

	This module contains firmware for an m68k-bus peripheral controller.


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */


/*
	Register definitions
*/
#define REG_DATA_A				(0)		/* Channel A RX/TX data register		*/
#define REG_STATUS_A			(1)		/* Channel A status flags				*/
#define REG_INT_CFG_A			(2)		/* Channel A interrupt-enable flags		*/
#define REG_CFG					(3)		/* Controller configuration				*/
#define REG_DATA_B				(4)		/* Channel B RX/TX data register		*/
#define REG_STATUS_B			(5)		/* Channel B status flags				*/
#define REG_INT_CFG_B			(6)		/* Channel B interrupt-enable flags		*/
#define REG_RESERVED			(7)

/* Flag bits - used in REG_STATUS and REG_INT_CFG */
#define FLAG_RX					_BV(7)	/* Byte received						*/
#define FLAG_TX					_BV(6)	/* Transmit finished					*/
#define FLAG_PAR_ERR			_BV(5)	/* Receiver parity error				*/
#define FLAG_OVF				_BV(4)	/* FIFO overflow						*/
#define FLAG_RX_TIMEOUT			_BV(3)	/* Timeout during receive				*/
#define FLAG_TX_TIMEOUT			_BV(2)	/* Timeout during transmit				*/
#define FLAG_START_TIMEOUT		_BV(1)	/* Timeout while waiting for TX clock	*/
#define FLAG_CMD_TIMEOUT		_BV(0)	/* Timeout waiting for command response	*/

/* Bits in REG_CFG */
#define CFG_IE					_BV(7)	/* Global interrupt enable				*/
#define CFG_PWR_A				_BV(4)	/* Enable/disable channel A power		*/
#define CFG_PWR_B				_BV(3)	/* Enable/disable channel B power		*/

#endif