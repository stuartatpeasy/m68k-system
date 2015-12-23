/*
	PS2Controller.h - dual-channel PS/2 controller implementation
	
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


void init();
void process_clock_edge(volatile ctx_t *ctxl);
void process_data(volatile ctx_t *ctx);
void timer_start(volatile ctx_t *ctx);
void timer_stop(volatile ctx_t *ctx);
void handle_timer_event(volatile ctx_t *ctx);
void start_tx();
inline void fifo_read(volatile ctx_t *ctx);
inline void try_start_tx(volatile ctx_t *ctx, ku8 data);
int main();

#endif