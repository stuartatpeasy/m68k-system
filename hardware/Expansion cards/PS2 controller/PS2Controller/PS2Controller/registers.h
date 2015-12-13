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
#define REG_KB_DATA			(0)
#define REG_KB_MODIFIERS	(1)
#define REG_MOUSE_DATA		(2)
#define REG_STATUS			(3)

/* Bits in REG_KB_MODIFIERS */
#define	MOD_LSHIFT			(7)
#define MOD_RSHIFT			(6)
#define MOD_LCTRL			(5)
#define MOD_RCTRL			(4)
#define MOD_LALT			(3)
#define MOD_RALT			(2)

/* Bits in REG_STATUS */
#define STATUS_KBRX			(7)
#define STATUS_MOUSERX		(6)



#endif