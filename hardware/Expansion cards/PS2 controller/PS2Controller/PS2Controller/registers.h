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
#define REG_KB_DATA				_BV(0)
#define REG_MOUSE_DATA			_BV(1)
#define REG_KB_MODIFIERS		_BV(2)
#define REG_STATUS				_BV(3)
#define REG_CFG					_BV(4)
#define REG_INTCFG				_BV(5)

/* Bits in REG_KB_MODIFIERS */
#define	MOD_LSHIFT				_BV(7)
#define MOD_LCTRL				_BV(6)
#define MOD_LGUI				_BV(5)
#define MOD_LALT				_BV(4)
#define MOD_RSHIFT				_BV(3)
#define MOD_RCTRL				_BV(2)
#define MOD_GUI					_BV(1)
#define MOD_RALT				_BV(0)

/* Bits in REG_STATUS */
#define STATUS_KBRX				_BV(6)
#define STATUS_KBTXDONE			_BV(5)
#define STATUS_KBPARERR			_BV(4)
#define STATUS_MOUSERX			_BV(3)
#define STATUS_MOUSETXDONE		_BV(2)
#define STATUS_MOUSEPARERR		_BV(1)

/* Bits in REG_CONFIG */
#define INTCFG_IE				_BV(7)	/* Global interrupt enable						*/
#define INTCFG_KBRXIE			_BV(6)	/* Keyboard data RX interrupt enable			*/
#define INTCFG_KBTXIE			_BV(5)	/* Keyboard data TX complete interrupt enable	*/
#define INTCFG_KBPARERRIE		_BV(4)	/* Keyboard RX parity error interrupt enable	*/
#define INTCFG_MOUSERXIE		_BV(3)	/* Mouse data RX interrupt enable				*/
#define INTCFG_MOUSETXIE		_BV(2)	/* Mouse data TX complete interrupt enable		*/
#define INTCFG_MOUSEPARERRIE	_BV(1)	/* Mouse RX parity error interrupt enable		*/

#endif