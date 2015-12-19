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

#define PERIPHERAL_ID		(0x82)		/* Peripheral identifier */

#define IRQ_EDGE_FALLING	(0)
#define IRQ_EDGE_RISING		(1)


typedef unsigned char	u8;
typedef const u8		ku8;
typedef volatile u8		vu8;


/* PS/2 data transmission/reception states */
typedef enum state
{
	state_idle		= 0,
	state_rx_start	= 1,
	state_rx_d0		= 2,
	state_rx_d1		= 3,
	state_rx_d2		= 4,
	state_rx_d3		= 5,
	state_rx_d4		= 6,
	state_rx_d5		= 7,
	state_rx_d6		= 8,
	state_rx_d7		= 9,
	state_rx_parity	= 10,
	state_rx_stop	= 11,
	
	state_tx_busrq	= 12,
	state_tx_start	= 13,
	state_tx_d0		= 14,
	state_tx_d1		= 15,
	state_tx_d2		= 16,
	state_tx_d3		= 17,
	state_tx_d4		= 18,
	state_tx_d5		= 19,
	state_tx_d6		= 20,
	state_tx_d7		= 21,
	state_tx_parity	= 22,
	state_tx_stop	= 23,
	state_tx_ack	= 24,
} state_t;


typedef enum irq_edge
{
	irq_edge_falling	= 0,
	irq_edge_rising		= 1
} irq_edge_t;


typedef enum irq
{
	irq_keyboard_clk	= 0,
	irq_mouse_clk		= 1
} irq_t;


/* PS/2 command values (sent from host to device) */
#define CMD_NONE			(0x00)
#define KB_CMD_SET_LEDS		(0xed)
#define KB_CMD_SET_TM_RATE	(0xf3)
#define KB_CMD_RESET		(0xff)


/* FIFO buffer structure - used for buffering received mouse/KB data */
typedef struct fifo
{
	u8				rd;
	u8				wr;
	u8				data[256];	/* Hardwired to 256 to simplify buffer-wrapping calcs */
} fifo_t;


/* Keyboard/mouse data transmission/reception context */
typedef struct ctx
{
	state_t			state;

	vu8 *			port;
	vu8 *			pin;
	vu8 *			ddr;
	u8				data_pin;
	u8				data;
	u8				parity_calc;
	u8				parity_received;
	u8				data_regnum;
	u8				command;
	u8				command_pending;
	void			(*start_tx_fn)(void);
	
	/* These members identify flags in REG_STATUS and REG_INTCFG */
	u8				flag_rx;		/* Data has been received			*/
	u8				flag_tx;		/* Data-transmission is complete	*/
	u8				flag_ovf;		/* FIFO overflow					*/
	u8				flag_parerr;	/* Receiver parity error			*/
	fifo_t			fifo;
} ctx_t;


void init();
void set_irq_edge(const irq_t irq, const irq_edge_t edge);
void process_clock_edge(volatile ctx_t *ctxl);
void process_data(volatile ctx_t *ctx);
void start_tx_keyboard();
void start_tx_mouse();
int main();

#endif