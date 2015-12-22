/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#ifndef PS2CONTROLLER_H_
#define PS2CONTROLLER_H_

#define F_CPU		8000000UL		/* 8MHz CPU clock */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "debug.h"
#include "macros.h"
#include "portdefs.h"
#include "registers.h"
#include "types.h"

#define PERIPHERAL_ID		(0x82)		/* Peripheral identifier */

#define IRQ_EDGE_FALLING	(0)
#define IRQ_EDGE_RISING		(1)


/* PS/2 command values (sent from host to device) */
#define CMD_NONE			(0x00)
#define KB_CMD_SET_LEDS		(0xed)
#define KB_CMD_SET_TM_RATE	(0xf3)
#define KB_CMD_RESET		(0xff)


void init();
void set_irq_edge(const irq_t irq, const irq_edge_t edge);
void process_clock_edge(volatile ctx_t *ctxl);
void process_data(volatile ctx_t *ctx);
void start_tx_keyboard();
void start_tx_mouse();
int main();

#endif